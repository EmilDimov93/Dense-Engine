// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "scene.hpp"

#include "modelLoader.hpp"

#include "../buffer.hpp"
#include "../assertions.hpp"

#include <cstring>

namespace DE
{
    Handle Scene::addModel(const std::string &filePath)
    {
        ModelUploadData modelUploadData = loadModel(filePath);

        Model model;

        model.meshes.reserve(modelUploadData.meshes.size());
        for (const MeshUploadData &meshUpload : modelUploadData.meshes)
            model.meshes.emplace_back(meshUpload, device, commandList);

        model.materials.reserve(modelUploadData.materials.size());
        for (const MaterialUploadData &materialUpload : modelUploadData.materials)
            model.materials.emplace_back(materialUpload.diffuse, materialUpload.metallic, materialUpload.roughness, -1);

        Handle handle = handleGenerator.generateHandle();

        models.emplace(handle, std::move(model));

        modelMaterialArrayOffsets.emplace(handle, 0);

        return handle;
    }

    Handle Scene::addModelInstance(Handle modelHandle, Transform transform)
    {
        Handle handle = handleGenerator.generateHandle();

        modelInstances.emplace(handle, ModelInstance(modelHandle, transform));

        return handle;
    }

    void Scene::upload()
    {
        uploadMaterials();
        buildTLAS();
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
