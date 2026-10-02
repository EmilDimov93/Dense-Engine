// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../buffer.hpp"

#include "../common.hpp"

#include "../assertions.hpp"

#include "fence.hpp"

namespace DE::abstractions
{
    struct AccelerationStructure
    {
        ComPtr<ID3D12Resource> buffer;
        ComPtr<ID3D12Resource> scratchBuffer;

        void build(ID3D12Device5 *device, ID3D12GraphicsCommandList4 *commandList, const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS &inputs)
        {
            D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuildInfo{};
            device->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &prebuildInfo);
            if (prebuildInfo.ResultDataMaxSizeInBytes == 0)
                fatal("Acceleration structure prebuild info returned zero size");

            const uint64_t sizeBytes = alignUp(prebuildInfo.ResultDataMaxSizeInBytes, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT);
            const uint64_t scratchSizeBytes = alignUp(prebuildInfo.ScratchDataSizeInBytes, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT);

            scratchBuffer = createBuffer(device, scratchSizeBytes, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
            buffer = createBuffer(device, sizeBytes, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

            D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc{};
            buildDesc.Inputs = inputs;
            buildDesc.ScratchAccelerationStructureData = scratchBuffer->GetGPUVirtualAddress();
            buildDesc.DestAccelerationStructureData = buffer->GetGPUVirtualAddress();

            commandList->BuildRaytracingAccelerationStructure(&buildDesc, 0, nullptr);

            D3D12_RESOURCE_BARRIER uavBarrier{};
            uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
            uavBarrier.UAV.pResource = buffer.Get();
            commandList->ResourceBarrier(1, &uavBarrier);
        }

        D3D12_GPU_VIRTUAL_ADDRESS getAddress() const { return buffer->GetGPUVirtualAddress(); }
    };
}