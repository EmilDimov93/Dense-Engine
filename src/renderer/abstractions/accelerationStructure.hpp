// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../common.hpp"

#include "../assertions.hpp"

#include "fence.hpp"

namespace DE::abstractions
{
    struct AccelerationStructure
    {
        ComPtr<ID3D12Resource> buffer;
        ComPtr<ID3D12Resource> scratchBuffer;

        void build(ID3D12Device5 *device, ID3D12GraphicsCommandList4 *commandList,
                   const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS &inputs);
        D3D12_GPU_VIRTUAL_ADDRESS getAddress() const { return buffer->GetGPUVirtualAddress(); }
    };
}