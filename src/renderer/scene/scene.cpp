// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "scene.hpp"

#include "modelLoader.hpp"

namespace DE
{
    Handle Scene::addModel(const std::string &filePath)
    {
        ModelUploadData modelUploadData = loadModel(filePath);

        Model model;

        model.meshes.reserve(modelUploadData.meshes.size());
        for(const MeshUploadData &meshUpload : modelUploadData.meshes)
            model.meshes.emplace_back(meshUpload, device, commandList);

        model.materials = std::move(modelUploadData.materials);

        Handle handle = handleGenerator.generateHandle();

        models.emplace(handle, std::move(model));

        return handle;
    }

    Handle Scene::addModelInstance(Handle modelHandle, Transform transform)
    {
        Handle handle = handleGenerator.generateHandle();

        modelInstances.emplace(handle, ModelInstance(modelHandle, transform));

        return handle;
    }
}
