// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "common.hpp"

#include "assertions.hpp"

namespace DE
{
    uint64_t alignUp(uint64_t sizeBytes, uint64_t alignmentBytes)
    {
        return (sizeBytes + alignmentBytes - 1) & ~(alignmentBytes - 1);
    }

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