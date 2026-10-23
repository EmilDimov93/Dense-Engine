// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "core/dxcommon.hpp"

#include "sceneDescription.hpp"

#include "model.hpp"
#include "modelInstance.hpp"

#include "pointLight.hpp"
#include "directionalLight.hpp"

#include <unordered_map>
#include <string>

namespace DE
{
    struct TLASInstanceData
    {
        uint32_t vertexOffset;
        uint32_t indexOffset;
        uint32_t materialIndex;
    };

    struct LightCounts
    {
        uint32_t pointLightCount, directionalLightCount;
    };

    struct SceneSettings
    {
        XMFLOAT3 clearColor = {0.f, 0.f, 0.f};
        float exposure = 1.f;
        XMFLOAT3 ambient = {0.2f, 0.2f, 0.2f};
    };

    class Scene
    {
    public:
        Scene(ID3D12Device5 *device, ID3D12GraphicsCommandList4 *commandList) : device(device), commandList(commandList) {}

        void upload(const SceneDescription &sceneDescription);
        D3D12_GPU_VIRTUAL_ADDRESS getMaterialBufferAddress() const { return materialBuffer->GetGPUVirtualAddress(); }
        D3D12_GPU_VIRTUAL_ADDRESS getVertexBufferAddress() const { return vertexBuffer->GetGPUVirtualAddress(); }
        D3D12_GPU_VIRTUAL_ADDRESS getIndexBufferAddress() const { return indexBuffer->GetGPUVirtualAddress(); }
        D3D12_GPU_VIRTUAL_ADDRESS getTLASInstanceDataAddress() const { return tlasInstanceDataBuffer->GetGPUVirtualAddress(); }
        D3D12_GPU_VIRTUAL_ADDRESS getTLASAddress() const { return tlas.getAddress(); }
        D3D12_GPU_VIRTUAL_ADDRESS getPointLightBufferAddress() const { return pointLightBuffer->GetGPUVirtualAddress(); }
        D3D12_GPU_VIRTUAL_ADDRESS getDirectionalLightBufferAddress() const { return directionalLightBuffer->GetGPUVirtualAddress(); }

        [[nodiscard]] bool empty() const { return modelInstances.empty(); }

        void clear();

        [[nodiscard]] LightCounts getLightCounts() const { return LightCounts{static_cast<uint32_t>(pointLights.size()), static_cast<uint32_t>(directionalLights.size())}; }

        [[nodiscard]] SceneSettings getSceneSettings() const { return sceneSettings; }

    private:
        std::unordered_map<Handle, Model> models;
        std::unordered_map<Handle, ModelInstance> modelInstances;

        std::unordered_map<Handle, PointLight> pointLights;
        std::unordered_map<Handle, DirectionalLight> directionalLights;

        std::unordered_map<Handle, uint32_t> modelMaterialArrayOffsets;

        SceneSettings sceneSettings;

        ComPtr<ID3D12Resource> instanceBuffer;

        ComPtr<ID3D12Resource> materialBuffer;

        ComPtr<ID3D12Resource> pointLightBuffer;
        ComPtr<ID3D12Resource> directionalLightBuffer;

        dxabstractions::AccelerationStructure tlas;

        ID3D12Device5 *device;
        ID3D12GraphicsCommandList4 *commandList;

        ComPtr<ID3D12Resource> vertexBuffer;
        ComPtr<ID3D12Resource> indexBuffer;
        ComPtr<ID3D12Resource> tlasInstanceDataBuffer;

        void uploadGeometry();
        void uploadMaterials();
        void uploadLights();
        void buildTLAS();
    };
}