// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "core/dxcommon.hpp"
#include "core/assertions.hpp"

#include <algorithm>
#include <cstring>
#include <span>

namespace DE
{
    inline uint64_t alignUp(uint64_t sizeBytes, uint64_t alignmentBytes)
    {
        return (sizeBytes + alignmentBytes - 1) & ~(alignmentBytes - 1);
    }

    inline ID3D12ResourcePtr createBuffer(ID3D12Device5 *device, UINT64 size, D3D12_HEAP_TYPE heapType, D3D12_RESOURCE_STATES initialState, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE)
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

        ID3D12ResourcePtr buffer;
        dxa(device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc, initialState, nullptr, IID_PPV_ARGS(&buffer)), "Failed to create buffer");
        return buffer;
    }

    inline D3D12_RESOURCE_BARRIER transition(ID3D12Resource *resource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after)
    {
        D3D12_RESOURCE_BARRIER barrier{};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = resource;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrier.Transition.StateBefore = before;
        barrier.Transition.StateAfter = after;
        return barrier;
    }

    template <typename T>
    [[nodiscard]] inline ID3D12ResourcePtr createUploadBuffer(ID3D12Device5 *device, std::span<const T> elements)
    {
        const UINT64 dataSizeBytes = static_cast<UINT64>(sizeof(T)) * elements.size();
        const UINT64 bufferSizeBytes = std::max<UINT64>(dataSizeBytes, sizeof(T));
        ID3D12ResourcePtr uploadBuffer = createBuffer(device, bufferSizeBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_FLAG_NONE);

        void *mappedData = nullptr;
        const D3D12_RANGE noCpuReadRange{0, 0};

        dxa(uploadBuffer->Map(0, &noCpuReadRange, &mappedData), "Failed to map upload buffer");

        std::memcpy(mappedData, elements.data(), dataSizeBytes);
        uploadBuffer->Unmap(0, nullptr);

        return uploadBuffer;
    }
}