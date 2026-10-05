// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../common.hpp"

#include "model.hpp"
#include "modelInstance.hpp"

#include <unordered_map>
#include <string>

namespace DE
{
    class Scene
    {
    public:
        Scene(ID3D12Device5 *device, ID3D12GraphicsCommandList4 *commandList) : device(device), commandList(commandList) {}

        Handle addModel(const std::string &filePath);
        Handle addModelInstance(Handle modelHandle, Transform transform);

        void buildTLAS();
        D3D12_GPU_VIRTUAL_ADDRESS getTLASAddress() const { return tlas.getAddress(); }

        void flattenMaterials();

    private:
        std::unordered_map<Handle, Model> models;
        std::unordered_map<Handle, ModelInstance> modelInstances;

        std::unordered_map<Handle, uint32_t> modelMaterialArrayOffsets;

        ComPtr<ID3D12Resource> instanceBuffer;

        ComPtr<ID3D12Resource> materialBuffer;

        abstractions::AccelerationStructure tlas;

        HandleGenerator handleGenerator;

        ID3D12Device5 *device;
        ID3D12GraphicsCommandList4 *commandList;
    };
}