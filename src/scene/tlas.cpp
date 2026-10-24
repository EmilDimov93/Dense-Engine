// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "scene.hpp"

#include "core/assertions.hpp"

#include "dxabstractions/buffer.hpp"

#include <cstring>

namespace DE
{
    void Scene::buildTLAS()
    {
        std::vector<D3D12_RAYTRACING_INSTANCE_DESC> instanceDescs;
        std::vector<TLASInstanceData> tlasInstanceData;

        for (const auto &entry : modelInstances)
        {
            const ModelInstance &instance = entry.second;

            auto modelIt = models.find(instance.modelHandle);
            if (modelIt == models.end())
                fatal("Model instance references unknown model");

            const uint32_t materialOffset = modelMaterialArrayOffsets.at(instance.modelHandle);

            const Transform &transform = instance.transform;
            XMMATRIX world = XMMatrixScaling(transform.scale.x, transform.scale.y, transform.scale.z) *
                             XMMatrixRotationRollPitchYaw(transform.rot.x, transform.rot.y, transform.rot.z) *
                             XMMatrixTranslation(transform.pos.x, transform.pos.y, transform.pos.z);

            for (const Mesh &mesh : modelIt->second.meshes)
            {
                D3D12_RAYTRACING_INSTANCE_DESC instanceDesc{};
                XMStoreFloat3x4(reinterpret_cast<XMFLOAT3X4 *>(instanceDesc.Transform), world);
                instanceDesc.InstanceID = entry.first;
                instanceDesc.InstanceMask = 0xFF;
                instanceDesc.InstanceContributionToHitGroupIndex = 0;
                instanceDesc.Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;
                instanceDesc.AccelerationStructure = mesh.blas.getAddress();

                instanceDescs.push_back(instanceDesc);
                tlasInstanceData.push_back({mesh.vertexOffset, mesh.indexOffset, materialOffset + mesh.materialIndex});
            }
        }

        if (instanceDescs.empty())
            fatal("Cannot build TLAS without model instances");

        instanceBuffer = createUploadBuffer<D3D12_RAYTRACING_INSTANCE_DESC>(device, instanceDescs);

        tlasInstanceDataBuffer = createUploadBuffer<TLASInstanceData>(device, tlasInstanceData);

        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS tlasInputs{};
        tlasInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
        tlasInputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
        tlasInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
        tlasInputs.NumDescs = static_cast<UINT>(instanceDescs.size());
        tlasInputs.InstanceDescs = instanceBuffer->GetGPUVirtualAddress();

        tlas.build(device, commandList, tlasInputs);
    }
}
