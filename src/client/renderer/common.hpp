// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

#include <DirectXMath.h>
using namespace DirectX;

#include <cstdint>
#include <array>

#include "assertions.hpp"

namespace DE
{
#ifdef _DEBUG
    inline constexpr bool DEBUG_MODE = true;
#else
    inline constexpr bool DEBUG_MODE = false;
#endif

    template <typename T>
    using ComPtr = Microsoft::WRL::ComPtr<T>;

    static constexpr uint32_t FRAMES_IN_FLIGHT = 2;

    template <typename T>
    using PerFrame = std::array<T, FRAMES_IN_FLIGHT>;

    inline ComPtr<ID3D12Resource> createBuffer(ID3D12Device5 *device, UINT64 size, D3D12_HEAP_TYPE heapType, D3D12_RESOURCE_STATES initialState, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE)
    {
        D3D12_HEAP_PROPERTIES heapProps{.Type = heapType};

        D3D12_RESOURCE_DESC bufferDesc{
            .Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
            .Alignment = 0,
            .Width = size,
            .Height = 1,
            .DepthOrArraySize = 1,
            .MipLevels = 1,
            .Format = DXGI_FORMAT_UNKNOWN,
            .SampleDesc = {1, 0},
            .Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
            .Flags = flags};

        ComPtr<ID3D12Resource> buffer;
        dxa(device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc, initialState, nullptr, IID_PPV_ARGS(&buffer)), "Failed to create buffer");
        return buffer;
    }
}