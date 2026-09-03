// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "assertions.hpp"
#include "common.hpp"

namespace DE
{
    struct Device
    {
        ComPtr<IDXGIFactory6> factory = nullptr;
        ComPtr<IDXGIAdapter1> adapter = nullptr;
        ComPtr<ID3D12Device5> device = nullptr;

        Device()
        {
            UINT factoryFlags = 0;
            if (DEBUG_MODE)
                factoryFlags |= DXGI_CREATE_FACTORY_DEBUG;

            dxa(CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&factory)), "Failed to create factory");

            for (UINT adapterIndex = 0; SUCCEEDED(factory->EnumAdapterByGpuPreference(adapterIndex, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter))); ++adapterIndex)
            {
                DXGI_ADAPTER_DESC1 adapterDesc{};
                adapter->GetDesc1(&adapterDesc);

                if (adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
                    continue;

                if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&device))))
                    break;
            }

            ptra(device, "Failed to find suitable adapter");
        }

        ~Device()
        {
            device.Reset();
            adapter.Reset();
            factory.Reset();
        }
    };
}