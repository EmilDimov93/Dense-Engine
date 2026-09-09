// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../assertions.hpp"
#include "../common.hpp"

#include "fence.hpp"

namespace DE
{
    struct CommandContext
    {
        ComPtr<ID3D12CommandQueue> queue = nullptr;
        ComPtr<ID3D12CommandAllocator> allocators[FRAMES_IN_FLIGHT] = {};
        ComPtr<ID3D12GraphicsCommandList4> list = nullptr;

        Fence fence;

        explicit CommandContext(ID3D12Device5 *device) : fence(device)
        {
            D3D12_COMMAND_QUEUE_DESC queueDesc
            {
                .Type = D3D12_COMMAND_LIST_TYPE_DIRECT,
                .Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
                .Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
                .NodeMask = 0
            };

            dxa(device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&queue)), "Failed to create command queue");

            for (uint32_t frameIndex = 0; frameIndex < FRAMES_IN_FLIGHT; ++frameIndex)
                dxa(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocators[frameIndex])), "Failed to create command allocator");

            dxa(device->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&list)), "Failed to create command list");
        }
    };
}