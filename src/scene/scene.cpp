// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "scene.hpp"

#include "modelLoader.hpp"

#include "core/assertions.hpp"

#include "dxabstractions/buffer.hpp"

#include <cstring>

namespace DE
{
    void Scene::upload(const SceneDescription &sceneDescription)
    {
        models.reserve(sceneDescription.modelDescriptions.size());
        for (const ModelDescription &modelDescription : sceneDescription.modelDescriptions)
            if (models.find(modelDescription.handle) == models.end())
            {
                ModelUploadData modelUploadData = loadModel(modelDescription.filePath);

                Model model;

                model.meshes.reserve(modelUploadData.meshes.size());
                for (const MeshUploadData &meshUpload : modelUploadData.meshes)
                    model.meshes.emplace_back(meshUpload, device, commandList);

                model.materials.reserve(modelUploadData.materials.size());
                for (const MaterialUploadData &materialUpload : modelUploadData.materials)
                    model.materials.emplace_back(materialUpload.diffuse, materialUpload.metallic, materialUpload.roughness, -1);

                models.emplace(modelDescription.handle, std::move(model));

                modelMaterialArrayOffsets.emplace(modelDescription.handle, 0);
            }

        modelInstances.reserve(sceneDescription.modelInstanceDescriptions.size());
        for (const ModelInstanceDescription &modelInstanceDescription : sceneDescription.modelInstanceDescriptions)
        {
            auto found = modelInstances.find(modelInstanceDescription.handle);
            if (found == modelInstances.end())
                modelInstances.emplace(modelInstanceDescription.handle, ModelInstance(modelInstanceDescription.modelHandle, modelInstanceDescription.transform));
            else
                found->second.transform = modelInstanceDescription.transform;
        }

        sceneSettings.clearColor = sceneDescription.clearColor;
        sceneSettings.exposure = sceneDescription.exposure;
        sceneSettings.ambient = sceneDescription.ambient;

        if (empty())
            return;

        uploadGeometry();
        uploadMaterials();
        buildTLAS();

        pointLights.clear();
        pointLights.reserve(sceneDescription.pointLightDescriptions.size());
        for (const PointLightDescription &pointLightDescription : sceneDescription.pointLightDescriptions)
        {
            auto found = pointLights.find(pointLightDescription.handle);
            if (found == pointLights.end())
                pointLights.emplace(pointLightDescription.handle, PointLight(pointLightDescription.pos, pointLightDescription.intensity, pointLightDescription.color, pointLightDescription.range));
            else
                found->second = PointLight(pointLightDescription.pos, pointLightDescription.intensity, pointLightDescription.color, pointLightDescription.range);
        }

        directionalLights.clear();
        directionalLights.reserve(sceneDescription.directionalLightDescriptions.size());
        for (const DirectionalLightDescription &directionalLightDescription : sceneDescription.directionalLightDescriptions)
        {
            auto found = directionalLights.find(directionalLightDescription.handle);
            if (found == directionalLights.end())
                directionalLights.emplace(directionalLightDescription.handle, DirectionalLight(directionalLightDescription.direction, directionalLightDescription.intensity, directionalLightDescription.color));
            else
                found->second = DirectionalLight(directionalLightDescription.direction, directionalLightDescription.intensity, directionalLightDescription.color);
        }

        uploadLights();
    }

    void Scene::clear()
    {
        models.clear();
        modelInstances.clear();
        pointLights.clear();
        directionalLights.clear();
        modelMaterialArrayOffsets.clear();

        vertexBuffer = nullptr;
        indexBuffer = nullptr;
        materialBuffer = nullptr;
        tlasInstanceDataBuffer = nullptr;
        instanceBuffer = nullptr;
        pointLightBuffer = nullptr;
        directionalLightBuffer = nullptr;
        tlas = {};
    }

    void Scene::uploadGeometry()
    {
        uint64_t totalVertexCount = 0;
        uint64_t totalIndexCount = 0;

        for (auto &[handle, model] : models)
            for (Mesh &mesh : model.meshes)
            {
                mesh.vertexOffset = static_cast<uint32_t>(totalVertexCount);
                mesh.indexOffset = static_cast<uint32_t>(totalIndexCount);
                totalVertexCount += mesh.vertexCount;
                totalIndexCount += mesh.indexCount;
            }

        vertexBuffer = createBuffer(device, sizeof(Vertex) * totalVertexCount, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_FLAG_NONE);
        indexBuffer = createBuffer(device, sizeof(uint32_t) * totalIndexCount, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_FLAG_NONE);

        for (const auto &[handle, model] : models)
            for (const Mesh &mesh : model.meshes)
            {
                commandList->CopyBufferRegion(vertexBuffer, sizeof(Vertex) * mesh.vertexOffset, mesh.vertexBuffer, 0, sizeof(Vertex) * mesh.vertexCount);
                commandList->CopyBufferRegion(indexBuffer, sizeof(uint32_t) * mesh.indexOffset, mesh.indexBuffer, 0, sizeof(uint32_t) * mesh.indexCount);
            }

        const D3D12_RESOURCE_BARRIER barriers[] = {
            transition(vertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE),
            transition(indexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE)};
        commandList->ResourceBarrier(static_cast<UINT>(std::size(barriers)), barriers);
    }

    void Scene::uploadMaterials()
    {
        std::vector<Material> materials;

        for (auto &[handle, model] : models)
        {
            modelMaterialArrayOffsets[handle] = static_cast<uint32_t>(materials.size());
            materials.insert(materials.end(), model.materials.begin(), model.materials.end());
        }

        materialBuffer = createUploadBuffer<Material>(device, materials);
    }

    void Scene::uploadLights()
    {
        // Point Lights
        std::vector<PointLight> pointLightsVector;
        pointLightsVector.reserve(pointLights.size());

        for (const auto &[handle, pointLight] : pointLights)
        {
            pointLightsVector.push_back(pointLight);
        }

        pointLightBuffer = createUploadBuffer<PointLight>(device, pointLightsVector);

        // Directional Lights
        std::vector<DirectionalLight> directionalLightsVector;
        directionalLightsVector.reserve(directionalLights.size());

        for (const auto &[handle, directionalLight] : directionalLights)
        {
            directionalLightsVector.push_back(directionalLight);
        }

        directionalLightBuffer = createUploadBuffer<DirectionalLight>(device, directionalLightsVector);
    }
}
