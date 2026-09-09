// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../assertions.hpp"
#include "../common.hpp"

namespace DE
{
    struct Fence
    {
        ComPtr<ID3D12Fence> fence = nullptr;
        HANDLE fenceEvent = nullptr;

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
    };
}