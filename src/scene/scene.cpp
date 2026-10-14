// Copyright 2025 Emil Dimov
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

        for (const ModelInstanceDescription &modelInstanceDescription : sceneDescription.modelInstanceDescriptions)
        {
            auto found = modelInstances.find(modelInstanceDescription.handle);
            if (found == modelInstances.end())
                modelInstances.emplace(modelInstanceDescription.handle, ModelInstance(modelInstanceDescription.modelHandle, modelInstanceDescription.transform));
            else
                found->second.transform = modelInstanceDescription.transform;
        }

        uploadGeometry();
        uploadMaterials();
        buildTLAS();
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
                commandList->CopyBufferRegion(vertexBuffer.Get(), sizeof(Vertex) * mesh.vertexOffset, mesh.vertexBuffer.Get(), 0, sizeof(Vertex) * mesh.vertexCount);
                commandList->CopyBufferRegion(indexBuffer.Get(), sizeof(uint32_t) * mesh.indexOffset, mesh.indexBuffer.Get(), 0, sizeof(uint32_t) * mesh.indexCount);
            }

        const D3D12_RESOURCE_BARRIER barriers[] = {
            transition(vertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE),
            transition(indexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE)};
    }

    void Scene::uploadMaterials()
    {
        std::vector<Material> materials;

        for (auto &[handle, model] : models)
        {
            modelMaterialArrayOffsets[handle] = static_cast<uint32_t>(materials.size());
            materials.insert(materials.end(), model.materials.begin(), model.materials.end());
        }

        {
            const uint64_t size = sizeof(Material) * materials.size();
            materialBuffer = createBuffer(device, size, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_FLAG_NONE);

            void *mappedData = nullptr;
            const D3D12_RANGE noCpuReadRange{0, 0};

            dxa(materialBuffer->Map(0, &noCpuReadRange, &mappedData), "Failed to map buffer");

            std::memcpy(mappedData, materials.data(), size);
            materialBuffer->Unmap(0, nullptr);
        }
    }
}
