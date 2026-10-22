// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "sceneDescription.hpp"

#include "core/assertions.hpp"
#include "version.hpp"

#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <iomanip>
#include <limits>
#include <filesystem>

namespace DE
{
    constexpr const char *endOfObject = "eoo";

    inline bool saveScene(const std::filesystem::path &filePath, const SceneDescription &sceneDescription)
    {
        std::ofstream outputFile(filePath);
        if (!outputFile.is_open())
        {
            warn("Failed to save scene");
            return false;
        }

        outputFile << std::setprecision(std::numeric_limits<float>::max_digits10);

        outputFile << DENSE_ENGINE_VERSION << '\n';

        outputFile << "Models" << '\n';

        for (const ModelDescription &modelDescription : sceneDescription.modelDescriptions)
        {
            outputFile << modelDescription.handle << '\n'
                       << modelDescription.filePath << '\n'
                       << endOfObject << '\n';
        }

        outputFile << "Instances" << '\n';

        for (const ModelInstanceDescription &instanceDescription : sceneDescription.modelInstanceDescriptions)
        {
            outputFile << instanceDescription.handle << '\n'
                       << instanceDescription.modelHandle << '\n'
                       << instanceDescription.transform.pos.x << '\n'
                       << instanceDescription.transform.pos.y << '\n'
                       << instanceDescription.transform.pos.z << '\n'
                       << instanceDescription.transform.rot.x << '\n'
                       << instanceDescription.transform.rot.y << '\n'
                       << instanceDescription.transform.rot.z << '\n'
                       << instanceDescription.transform.scale.x << '\n'
                       << instanceDescription.transform.scale.y << '\n'
                       << instanceDescription.transform.scale.z << '\n'
                       << endOfObject << '\n';
        }

        outputFile << "PointLights" << '\n';

        for (const PointLightDescription &pointLightDescription : sceneDescription.pointLightDescriptions)
        {
            outputFile << pointLightDescription.handle << '\n'
                       << pointLightDescription.pos.x << '\n'
                       << pointLightDescription.pos.y << '\n'
                       << pointLightDescription.pos.z << '\n'
                       << pointLightDescription.intensity << '\n'
                       << pointLightDescription.color.x << '\n'
                       << pointLightDescription.color.y << '\n'
                       << pointLightDescription.color.z << '\n'
                       << pointLightDescription.range << '\n'
                       << endOfObject << '\n';
        }

        outputFile << "DirectionalLights" << '\n';

        for (const DirectionalLightDescription &directionalLightDescription : sceneDescription.directionalLightDescriptions)
        {
            outputFile << directionalLightDescription.handle << '\n'
                       << directionalLightDescription.direction.x << '\n'
                       << directionalLightDescription.direction.y << '\n'
                       << directionalLightDescription.direction.z << '\n'
                       << directionalLightDescription.intensity << '\n'
                       << directionalLightDescription.color.x << '\n'
                       << directionalLightDescription.color.y << '\n'
                       << directionalLightDescription.color.z << '\n'
                       << endOfObject << '\n';
        }

        outputFile.close();
        if (outputFile.fail())
        {
            warn("Failed to save scene");
            return false;
        }

        return true;
    }

    inline bool loadScene(const std::filesystem::path &filePath, SceneDescription &sceneDescription, Handle &lastHandle)
    {
        std::ifstream inputFile(filePath);
        if (!inputFile.is_open())
        {
            warn("Failed to load scene file");
            return false;
        }

        SceneDescription newSceneDescription;

        try
        {
            std::string line;
            if (!std::getline(inputFile, line))
            {
                warn("Scene file is empty");
                return false;
            }
            else
            {
                if (stoul(line) != DENSE_ENGINE_VERSION)
                {
                    warn("Scene file version mismatch. Scene(" + line + "). Client version (" + std::to_string(DENSE_ENGINE_VERSION) + ")");
                }
            }

            enum Section
            {
                Context = 0u,
                Models = 1u,
                Instances = 2u,
                PointLights = 3u,
                DirectionalLights = 4u
            };
            Section currSection = Section::Context;
            uint32_t tokenCounter = 0;
            ModelDescription currModel;
            ModelInstanceDescription currInstance;
            PointLightDescription currPointLight;
            DirectionalLightDescription currDirectionalLight;
            while (std::getline(inputFile, line))
            {
                if (line == "Models")
                {
                    currSection = Section::Models;
                    tokenCounter = 0;
                    continue;
                }
                else if (line == "Instances")
                {
                    currSection = Section::Instances;
                    tokenCounter = 0;
                    continue;
                }
                else if (line == "PointLights")
                {
                    currSection = Section::PointLights;
                    tokenCounter = 0;
                    continue;
                }
                else if (line == "DirectionalLights")
                {
                    currSection = Section::DirectionalLights;
                    tokenCounter = 0;
                    continue;
                }

                switch (currSection)
                {
                case Section::Models:
                {
                    if (line == endOfObject)
                    {
                        newSceneDescription.modelDescriptions.push_back(currModel);
                        tokenCounter = 0;
                        continue;
                    }
                    else
                    {
                        switch (tokenCounter)
                        {
                        case 0:
                            currModel.handle = stoul(line);
                            break;
                        case 1:
                            currModel.filePath = line;
                            break;
                        }
                    }
                    break;
                }
                case Section::Instances:
                {
                    if (line == endOfObject)
                    {
                        newSceneDescription.modelInstanceDescriptions.push_back(currInstance);
                        tokenCounter = 0;
                        continue;
                    }
                    else
                    {
                        switch (tokenCounter)
                        {
                        case 0:
                            currInstance.handle = stoul(line);
                            break;
                        case 1:
                            currInstance.modelHandle = stoul(line);
                            break;
                        case 2:
                            currInstance.transform.pos.x = stof(line);
                            break;
                        case 3:
                            currInstance.transform.pos.y = stof(line);
                            break;
                        case 4:
                            currInstance.transform.pos.z = stof(line);
                            break;
                        case 5:
                            currInstance.transform.rot.x = stof(line);
                            break;
                        case 6:
                            currInstance.transform.rot.y = stof(line);
                            break;
                        case 7:
                            currInstance.transform.rot.z = stof(line);
                            break;
                        case 8:
                            currInstance.transform.scale.x = stof(line);
                            break;
                        case 9:
                            currInstance.transform.scale.y = stof(line);
                            break;
                        case 10:
                            currInstance.transform.scale.z = stof(line);
                            break;
                        }
                    }
                    break;
                }
                case Section::PointLights:
                {
                    if (line == endOfObject)
                    {
                        newSceneDescription.pointLightDescriptions.push_back(currPointLight);
                        tokenCounter = 0;
                        continue;
                    }
                    else
                    {
                        switch (tokenCounter)
                        {
                        case 0:
                            currPointLight.handle = stoul(line);
                            break;
                        case 1:
                            currPointLight.pos.x = stof(line);
                            break;
                        case 2:
                            currPointLight.pos.y = stof(line);
                            break;
                        case 3:
                            currPointLight.pos.z = stof(line);
                            break;
                        case 4:
                            currPointLight.intensity = stof(line);
                            break;
                        case 5:
                            currPointLight.color.x = stof(line);
                            break;
                        case 6:
                            currPointLight.color.y = stof(line);
                            break;
                        case 7:
                            currPointLight.color.z = stof(line);
                            break;
                        case 8:
                            currPointLight.range = stof(line);
                            break;
                        }
                    }
                    break;
                }
                case Section::DirectionalLights:
                {
                    if (line == endOfObject)
                    {
                        newSceneDescription.directionalLightDescriptions.push_back(currDirectionalLight);
                        tokenCounter = 0;
                        continue;
                    }
                    else
                    {
                        switch (tokenCounter)
                        {
                        case 0:
                            currDirectionalLight.handle = stoul(line);
                            break;
                        case 1:
                            currDirectionalLight.direction.x = stof(line);
                            break;
                        case 2:
                            currDirectionalLight.direction.y = stof(line);
                            break;
                        case 3:
                            currDirectionalLight.direction.z = stof(line);
                            break;
                        case 4:
                            currDirectionalLight.intensity = stof(line);
                            break;
                        case 5:
                            currDirectionalLight.color.x = stof(line);
                            break;
                        case 6:
                            currDirectionalLight.color.y = stof(line);
                            break;
                        case 7:
                            currDirectionalLight.color.z = stof(line);
                            break;
                        }
                    }
                    break;
                }
                default:
                {
                    continue;
                }
                }

                tokenCounter++;
            }
        }
        catch (const std::exception &)
        {
            warn("Failed to load scene: corrupted file");
            return false;
        }

        for(const ModelDescription &modelDescription : newSceneDescription.modelDescriptions)
            if(modelDescription.handle > lastHandle)
                lastHandle = modelDescription.handle;
        for(const ModelInstanceDescription &instanceDescription : newSceneDescription.modelInstanceDescriptions)
            if(instanceDescription.handle > lastHandle)
                lastHandle = instanceDescription.handle;
        for(const PointLightDescription &pointLightDescription : newSceneDescription.pointLightDescriptions)
            if(pointLightDescription.handle > lastHandle)
                lastHandle = pointLightDescription.handle;
        for(const DirectionalLightDescription &directionalLightDescription : newSceneDescription.directionalLightDescriptions)
            if(directionalLightDescription.handle > lastHandle)
                lastHandle = directionalLightDescription.handle;

        sceneDescription = std::move(newSceneDescription);

        return true;
    }
}