// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "dxc.hpp"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>
#include <cstdint>
#include <string>
#include <iostream>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

template <typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

namespace DE
{
    class Device
    {
    public:
        ComPtr<IDXGIFactory6> factory = nullptr;
        ComPtr<IDXGIAdapter1> adapter = nullptr;
        ComPtr<IDXGIDevice4> device = nullptr;

        Device()
        {
            // Factory
            UINT factoryFlags = 0;
            if (_DEBUG)
                factoryFlags |= DXGI_CREATE_FACTORY_DEBUG;

            dxc(CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&factory)), "Failed to create factory");

            // Adapter
            for (UINT adapterIndex = 0; SUCCEEDED(factory->EnumAdapterByGpuPreference(adapterIndex, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter))); ++adapterIndex)
            {
                DXGI_ADAPTER_DESC1 adapterDesc{};
                adapter->GetDesc1(&adapterDesc);

                if (adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
                    continue;

                if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), nullptr)))
                    break;
            }

            if (!adapter)
                assertExists(adapter, "Failed to find suitable adapter");
        }

        ~Device()
        {
            factory.Reset();
            adapter.Reset();
        }

    private:
    };
}