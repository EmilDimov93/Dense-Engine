// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../common.hpp"

namespace DE::abstractions
{
    struct RenderTarget
    {
        ComPtr<ID3D12Resource> texture;
        ComPtr<ID3D12DescriptorHeap> descriptorHeap;

        RenderTarget(ID3D12Device5 *device, UINT width, UINT height)
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

            D3D12_DESCRIPTOR_HEAP_DESC heapDesc{
                .Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
                .NumDescriptors = 1,
                .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
                .NodeMask = 0};

            dxa(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&descriptorHeap)), "Failed to create descriptor heap");

            D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{
                .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
                .ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D,
                .Texture2D = {0, 0}};

            device->CreateUnorderedAccessView(texture.Get(), nullptr, &uavDesc, descriptorHeap->GetCPUDescriptorHandleForHeapStart());
        }
    };
}
