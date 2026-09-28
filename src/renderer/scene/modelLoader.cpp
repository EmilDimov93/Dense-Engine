// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "modelLoader.hpp"

#include "../assertions.hpp"

#include <fstream>
#include <filesystem>
#include <sstream>
#include <unordered_map>

#define STB_IMAGE_IMPLEMENTATION
#include "../../../ext/stb_image/stb_image.h"

namespace DE
{
    [[nodiscard]] ModelUploadData loadOBJ(const std::string &filePath)
    {
        std::ifstream file(filePath);
        if (!file.is_open())
            fatal("Failed to open OBJ file");

        ModelUploadData result;
        std::unordered_map<std::string, uint32_t> materialIndexByName;

        std::vector<XMFLOAT3> positions;
        std::vector<XMFLOAT2> texCoords;
        std::vector<XMFLOAT3> normals;

        uint32_t currentMaterialIndex = UINT32_MAX;
        uint32_t defaultMaterialIndex = UINT32_MAX;

        std::vector<Vertex> currentMeshVertices;
        std::vector<uint32_t> currentMeshIndices;

        auto readRestOfLine = [](std::stringstream &ss) -> std::string
        {
            std::string rest;
            std::getline(ss >> std::ws, rest);
            rest.erase(rest.find_last_not_of(" \t\r\n") + 1);
            return rest;
        };

        auto loadMTL = [&](const std::filesystem::path &mtlPath)
        {
            std::ifstream mtl(mtlPath);
            if (!mtl.is_open())
            {
                warn("Failed to open MTL file");
                return;
            }

            Material *currentMaterial = nullptr;
            std::string line;
            while (std::getline(mtl, line))
            {
                std::stringstream ss(line);
                std::string keyword;
                ss >> keyword;

                if (keyword == "newmtl")
                {
                    materialIndexByName[readRestOfLine(ss)] = static_cast<uint32_t>(result.materials.size());
                    currentMaterial = &result.materials.emplace_back();
                }
                else if (!currentMaterial)
                    continue;
                else if (keyword == "Kd")
                {
                    XMFLOAT3 kdSrgb{};
                    ss >> kdSrgb.x >> kdSrgb.y >> kdSrgb.z;
                    XMVECTOR kdLinear = XMColorSRGBToRGB(XMVectorSet(kdSrgb.x, kdSrgb.y, kdSrgb.z, currentMaterial->diffuse.w));
                    XMStoreFloat4(&currentMaterial->diffuse, kdLinear);
                }
                else if (keyword == "map_Kd")
                {
                    std::filesystem::path texturePath = mtlPath.parent_path() / readRestOfLine(ss);

                    int width = 0;
                    int height = 0;
                    int channelsInFile = 0;
                    stbi_uc *decodedPixels = stbi_load(texturePath.string().c_str(), &width, &height, &channelsInFile, STBI_rgb_alpha);

                    if (!decodedPixels)
                    {
                        warn("Failed to load diffuse texture (map_Kd)");
                        continue;
                    }

                    currentMaterial->textureSize = XMUINT2(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
                    currentMaterial->texture.assign(decodedPixels, decodedPixels + static_cast<size_t>(width) * height * 4);
                    stbi_image_free(decodedPixels);
                }
                else if (keyword == "d")
                    ss >> currentMaterial->diffuse.w;
                else if (keyword == "Tr")
                {
                    float transparency = 0.f;
                    ss >> transparency;
                    currentMaterial->diffuse.w = 1.f - transparency;
                }
                else if (keyword == "Pm")
                    ss >> currentMaterial->metallic;
                else if (keyword == "Pr")
                    ss >> currentMaterial->roughness;
            }
        };

        auto finalizeCurrentMesh = [&]()
        {
            if (currentMeshVertices.empty())
                return;

            if (currentMaterialIndex == UINT32_MAX)
            {
                if (defaultMaterialIndex == UINT32_MAX)
                {
                    defaultMaterialIndex = static_cast<uint32_t>(result.materials.size());
                    result.materials.push_back(Material());
                }
                currentMaterialIndex = defaultMaterialIndex;
            }

            result.meshes.push_back({std::move(currentMeshVertices), std::move(currentMeshIndices), currentMaterialIndex});

            currentMeshVertices.clear();
            currentMeshIndices.clear();
        };

        auto resolveIndex = [](const std::string &rawText, size_t elementCount) -> int32_t
        {
            if (rawText.empty())
                return -1;
            int32_t rawIndex = std::stoi(rawText);
            int64_t resolvedIndex = rawIndex > 0 ? static_cast<int64_t>(rawIndex) - 1 : static_cast<int64_t>(elementCount) + rawIndex;
            if (rawIndex == 0 || resolvedIndex < 0 || resolvedIndex >= static_cast<int64_t>(elementCount))
                return -1;
            return static_cast<int32_t>(resolvedIndex);
        };

        struct Corner
        {
            int32_t positionIndex;
            int32_t texCoordIndex;
            int32_t normalIndex;
        };

        std::filesystem::path objPath(filePath);

        std::string line;
        while (std::getline(file, line))
        {
            std::stringstream ss(line);
            std::string keyword;
            ss >> keyword;

            if (keyword == "mtllib")
            {
                loadMTL(objPath.parent_path() / readRestOfLine(ss));
            }
            else if (keyword == "usemtl")
            {
                finalizeCurrentMesh();
                auto it = materialIndexByName.find(readRestOfLine(ss));
                currentMaterialIndex = it != materialIndexByName.end() ? it->second : UINT32_MAX;
            }
            else if (keyword == "o")
            {
                finalizeCurrentMesh();
            }
            else if (keyword == "v")
            {
                XMFLOAT3 position{};
                ss >> position.x >> position.y >> position.z;
                positions.push_back(position);
            }
            else if (keyword == "vt")
            {
                XMFLOAT2 texCoord{};
                ss >> texCoord.x >> texCoord.y;
                texCoords.push_back(texCoord);
            }
            else if (keyword == "vn")
            {
                XMFLOAT3 normal{};
                ss >> normal.x >> normal.y >> normal.z;
                normals.push_back(normal);
            }
            else if (keyword == "f")
            {
                Corner corners[3];
                bool faceIsValid = true;
                for (Corner &corner : corners)
                {
                    std::string token;
                    ss >> token;

                    std::stringstream tokenStream(token);
                    std::string positionText, texCoordText, normalText;
                    std::getline(tokenStream, positionText, '/');
                    std::getline(tokenStream, texCoordText, '/');
                    std::getline(tokenStream, normalText);

                    corner = {resolveIndex(positionText, positions.size()),
                              resolveIndex(texCoordText, texCoords.size()),
                              resolveIndex(normalText, normals.size())};

                    if (corner.positionIndex < 0)
                        faceIsValid = false;
                }

                if (!faceIsValid)
                    continue;

                XMVECTOR position0 = XMLoadFloat3(&positions[corners[0].positionIndex]);
                XMVECTOR position1 = XMLoadFloat3(&positions[corners[1].positionIndex]);
                XMVECTOR position2 = XMLoadFloat3(&positions[corners[2].positionIndex]);

                XMVECTOR faceNormalVector = XMVector3Cross(XMVectorSubtract(position1, position0), XMVectorSubtract(position2, position0));
                if (XMVectorGetX(XMVector3LengthSq(faceNormalVector)) < 1e-20f)
                    continue;

                XMFLOAT3 faceNormal;
                XMStoreFloat3(&faceNormal, XMVector3Normalize(faceNormalVector));

                for (const Corner &corner : corners)
                {
                    XMFLOAT2 tex = corner.texCoordIndex >= 0 ? texCoords[corner.texCoordIndex] : XMFLOAT2(0.f, 0.f);
                    tex.y = 1.f - tex.y;
                    XMFLOAT3 normal = corner.normalIndex >= 0 ? normals[corner.normalIndex] : faceNormal;

                    currentMeshIndices.push_back(static_cast<uint32_t>(currentMeshVertices.size()));
                    currentMeshVertices.emplace_back(positions[corner.positionIndex], normal, tex);
                }
            }
        }

        finalizeCurrentMesh();

        return result;
    }

    [[nodiscard]] ModelUploadData loadModel(const std::string &filePath)
    {
        std::string ext = std::filesystem::path(filePath).extension().string();

        ModelUploadData data;

        if (ext == ".obj")
        {
            data = loadOBJ(filePath);
        }
        else
        {
            fatal("Failed to load model: extention not supported");
            return data;
        }

        return data;
    }
}