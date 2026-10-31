// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "core/dxcommon.hpp"
#include "core/assertions.hpp"

#include "buffer.hpp"
#include "resourceBarrier.hpp"

namespace DE::dxabstractions
{
    constexpr DXGI_FORMAT TEXTURE_FORMAT = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

    struct Texture
    {
        ID3D12ResourcePtr resource;
        ID3D12ResourcePtr uploadBuffer;

        Texture(ID3D12Device5 *device, ID3D12GraphicsCommandList4 *commandList, std::span<const uint8_t> pixels, XMUINT2 size)
        {
            D3D12_HEAP_PROPERTIES heapProps{.Type = D3D12_HEAP_TYPE_DEFAULT};

            D3D12_RESOURCE_DESC textureDesc{
                .Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
                .Alignment = 0,
                .Width = size.x,
                .Height = size.y,
                .DepthOrArraySize = 1,
                .MipLevels = 1,
                .Format = TEXTURE_FORMAT,
                .SampleDesc = {1, 0},
                .Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
                .Flags = D3D12_RESOURCE_FLAG_NONE};

            dxa(device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &textureDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&resource)), "Failed to create texture");

            D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint{};
            UINT rowCount = 0;
            UINT64 rowSizeBytes = 0;
            UINT64 uploadSizeBytes = 0;
            device->GetCopyableFootprints(&textureDesc, 0, 1, 0, &footprint, &rowCount, &rowSizeBytes, &uploadSizeBytes);

            uploadBuffer = createBuffer(device, uploadSizeBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ);

            uint8_t *mappedData = nullptr;
            const D3D12_RANGE noCpuReadRange{0, 0};

            dxa(uploadBuffer->Map(0, &noCpuReadRange, reinterpret_cast<void **>(&mappedData)), "Failed to map texture upload buffer");

            for (UINT row = 0; row < rowCount; row++)
                std::memcpy(mappedData + footprint.Offset + row * footprint.Footprint.RowPitch, pixels.data() + row * rowSizeBytes, rowSizeBytes);
            uploadBuffer->Unmap(0, nullptr);

            D3D12_TEXTURE_COPY_LOCATION dst{};
            dst.pResource = resource;
            dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
            dst.SubresourceIndex = 0;

            D3D12_TEXTURE_COPY_LOCATION src{};
            src.pResource = uploadBuffer;
            src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
            src.PlacedFootprint = footprint;

            commandList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

            dxabstractions::barrier(resource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, commandList);
        }
    };
}