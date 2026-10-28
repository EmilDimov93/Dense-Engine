// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "core/dxcommon.hpp"
#include "core/assertions.hpp"

namespace DE::dxabstractions
{
    constexpr UINT DESCRIPTOR_HEAP_OUTPUT_UAV_INDEX = 0u;
    constexpr UINT DESCRIPTOR_HEAP_FIRST_TEXTURE_INDEX = 1u;

    constexpr UINT MAX_TEXTURES = 4096;
    constexpr UINT DESCRIPTOR_HEAP_SIZE = DESCRIPTOR_HEAP_FIRST_TEXTURE_INDEX + MAX_TEXTURES;

    class DescriptorHeap
    {
    public:
        DescriptorHeap(ID3D12Device5 *device)
        {
            D3D12_DESCRIPTOR_HEAP_DESC heapDesc{
                .Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
                .NumDescriptors = DESCRIPTOR_HEAP_SIZE,
                .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
                .NodeMask = 0};

            dxa(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&descriptorHeap)), "Failed to create descriptor heap");

            descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        }

        [[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle(UINT index) const
        {
            D3D12_CPU_DESCRIPTOR_HANDLE handle = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
            handle.ptr += static_cast<SIZE_T>(index) * descriptorSize;
            return handle;
        }

        [[nodiscard]] D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle(UINT index) const
        {
            D3D12_GPU_DESCRIPTOR_HANDLE handle = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
            handle.ptr += static_cast<UINT64>(index) * descriptorSize;
            return handle;
        }

        [[nodiscard]] ID3D12DescriptorHeap *get() const
        {
            return descriptorHeap;
        }

    private:
        ID3D12DescriptorHeapPtr descriptorHeap;
        UINT descriptorSize = 0;
    };
}
