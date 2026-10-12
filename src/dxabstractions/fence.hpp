// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "core/dxcommon.hpp"
#include "core/assertions.hpp"

namespace DE::dxabstractions
{
    struct Fence
    {
        ComPtr<ID3D12Fence> fence = nullptr;
        HANDLE fenceEvent = nullptr;
        UINT64 signaledValue = 0;

        explicit Fence(ID3D12Device5 *device)
        {
            dxa(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)), "Failed to create fence");

            fenceEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
            hndla(fenceEvent, "Failed to create fence event");
        }

        ~Fence()
        {
            if(fenceEvent)
                CloseHandle(fenceEvent);
        }

        void signal(ID3D12CommandQueue *queue)
        {
            dxa(queue->Signal(fence.Get(), ++signaledValue), "Failed to signal fence");
        }

        void wait()
        {
            if (fence->GetCompletedValue() >= signaledValue)
                return;

            dxa(fence->SetEventOnCompletion(signaledValue, fenceEvent), "Failed to set fence event");
            WaitForSingleObject(fenceEvent, INFINITE);
        }
    };
}
