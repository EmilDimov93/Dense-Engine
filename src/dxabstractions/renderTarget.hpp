// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "core/dxcommon.hpp"
#include "core/assertions.hpp"

namespace DE::dxabstractions
{
    struct RenderTarget
    {
        ID3D12ResourcePtr texture;

        RenderTarget(ID3D12Device5 *device, D3D12_CPU_DESCRIPTOR_HANDLE descriptorHeapHandle, UINT width, UINT height)
        {
            D3D12_HEAP_PROPERTIES heapProps{.Type = D3D12_HEAP_TYPE_DEFAULT};

            D3D12_RESOURCE_DESC textureDesc{
                .Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
                .Alignment = 0,
                .Width = width,
                .Height = height,
                .DepthOrArraySize = 1,
                .MipLevels = 1,
                .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
                .SampleDesc = {1, 0},
                .Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
                .Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS};

            dxa(device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &textureDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(&texture)), "Failed to create render target texture");

            D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{
                .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
                .ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D,
                .Texture2D = {0, 0}};

            device->CreateUnorderedAccessView(texture, nullptr, &uavDesc, descriptorHeapHandle);
        }
    };
}
