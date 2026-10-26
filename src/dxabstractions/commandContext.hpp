// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "core/dxcommon.hpp"
#include "core/assertions.hpp"

#include "fence.hpp"

namespace DE::dxabstractions
{
    class CommandContext
    {
    public:
        ID3D12CommandQueuePtr queue;
        ID3D12GraphicsCommandList4Ptr list;

        explicit CommandContext(ID3D12Device5 *device) : fences{Fence(device), Fence(device)}
        {
            D3D12_COMMAND_QUEUE_DESC queueDesc{
                .Type = D3D12_COMMAND_LIST_TYPE_DIRECT,
                .Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
                .Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
                .NodeMask = 0};

            dxa(device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&queue)), "Failed to create command queue");

            for (uint32_t frameIndex = 0; frameIndex < FRAMES_IN_FLIGHT; ++frameIndex)
                dxa(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocators[frameIndex])), "Failed to create command allocator");

            dxa(device->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&list)), "Failed to create command list");
        }

        void begin(uint32_t frameIndex)
        {
            fences[frameIndex].wait();

            ID3D12CommandAllocator *allocator = allocators[frameIndex];
            dxa(allocator->Reset(), "Failed to reset command allocator");
            dxa(list->Reset(allocator, nullptr), "Failed to reset command list");
        }

        void execute(uint32_t frameIndex)
        {
            dxa(list->Close(), "Failed to close command list");

            ID3D12CommandList *commandListsToExecute[] = {list};
            queue->ExecuteCommandLists(static_cast<UINT>(std::size(commandListsToExecute)), commandListsToExecute);

            fences[frameIndex].signal(queue);
        }

        void waitIdle()
        {
            for (dxabstractions::Fence &fence : fences)
                fence.wait();
        }

        void flush()
        {
            fences[0].signal(queue);
            fences[0].wait();
        }

    private:
        PerFrame<ID3D12CommandAllocatorPtr> allocators;

        PerFrame<Fence> fences;
    };
}