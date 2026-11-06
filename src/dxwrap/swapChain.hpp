// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "core/dxcommon.hpp"
#include "core/assertions.hpp"

#include "device.hpp"

namespace DE::dxwrap
{
    class SwapChain
    {
    public:
        SwapChain(Device &device, ID3D12CommandQueue *graphicsQueue, HWND windowHandle, UINT width, UINT height)
        {
            dxa(device.factory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &isTearingAllowed, sizeof(isTearingAllowed)), "Failed to check feature support");

            DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
            swapChainDesc.Width = width;
            swapChainDesc.Height = height;
            swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            swapChainDesc.Stereo = FALSE;
            swapChainDesc.SampleDesc = {1, 0};
            swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            swapChainDesc.BufferCount = FRAMES_IN_FLIGHT;
            swapChainDesc.Scaling = DXGI_SCALING_NONE;
            swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
            swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
            swapChainDesc.Flags = isTearingAllowed ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

            IDXGISwapChain1Ptr swapChain1;
            dxa(device.factory->CreateSwapChainForHwnd(graphicsQueue, windowHandle, &swapChainDesc, nullptr, nullptr, &swapChain1), "Failed to create swap chain");

            dxa(device.factory->MakeWindowAssociation(windowHandle, DXGI_MWA_NO_ALT_ENTER), "Failed to make window association");
            dxa(swapChain1->QueryInterface(IID_PPV_ARGS(&swapChain)), "Failed to copy swap chain");

            for (UINT bufferIndex = 0; bufferIndex < FRAMES_IN_FLIGHT; ++bufferIndex)
                dxa(swapChain->GetBuffer(bufferIndex, IID_PPV_ARGS(&backBuffers[bufferIndex])), "Failed to get swap chain buffer");

            size.x = width;
            size.y = height;

            D3D12_DESCRIPTOR_HEAP_DESC backBufferViewHeapDesc{};
            backBufferViewHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            backBufferViewHeapDesc.NumDescriptors = FRAMES_IN_FLIGHT;
            dxa(device.device->CreateDescriptorHeap(&backBufferViewHeapDesc, IID_PPV_ARGS(&backBufferViewHeap)), "Failed to create back buffer view heap");
            backBufferViewSize = device.device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

            for (UINT bufferIndex = 0; bufferIndex < FRAMES_IN_FLIGHT; ++bufferIndex)
                device.device->CreateRenderTargetView(backBuffers[bufferIndex], nullptr, backBufferView(bufferIndex));
        }

        D3D12_CPU_DESCRIPTOR_HANDLE backBufferView(UINT frameIndex) const
        {
            D3D12_CPU_DESCRIPTOR_HANDLE handle = backBufferViewHeap->GetCPUDescriptorHandleForHeapStart();
            handle.ptr += static_cast<SIZE_T>(frameIndex) * backBufferViewSize;
            return handle;
        }

        ID3D12ResourcePtr backBuffer(UINT frameIndex) const
        {
            return backBuffers[frameIndex];
        }

        void present(bool vsync)
        {
            if (vsync)
                dxa(swapChain->Present(1, 0), "Failed to present");
            else
                dxa(swapChain->Present(0, isTearingAllowed ? DXGI_PRESENT_ALLOW_TEARING : 0), "Failed to present");
        }

        void resize(XMUINT2 newSize, ID3D12Device5 *device)
        {
            for (UINT bufferIndex = 0; bufferIndex < FRAMES_IN_FLIGHT; ++bufferIndex)
                backBuffers[bufferIndex] = nullptr;

            dxa(swapChain->ResizeBuffers(FRAMES_IN_FLIGHT, newSize.x, newSize.y, DXGI_FORMAT_UNKNOWN, isTearingAllowed ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0), "Failed to resize swap chain");

            for (UINT bufferIndex = 0; bufferIndex < FRAMES_IN_FLIGHT; ++bufferIndex)
            {
                dxa(swapChain->GetBuffer(bufferIndex, IID_PPV_ARGS(&backBuffers[bufferIndex])), "Failed to get swap chain buffer");
                device->CreateRenderTargetView(backBuffers[bufferIndex], nullptr, backBufferView(bufferIndex));
            }

            size = newSize;
        }

        [[nodiscard]] UINT getCurrentBackBufferIndex() const
        {
            return swapChain->GetCurrentBackBufferIndex();
        }

        [[nodiscard]] XMUINT2 getSize() const
        {
            return size;
        }

    private:
        IDXGISwapChain4Ptr swapChain;

        PerFrame<ID3D12ResourcePtr> backBuffers;
        ID3D12DescriptorHeapPtr backBufferViewHeap;
        UINT backBufferViewSize = 0;

        XMUINT2 size{};
        BOOL isTearingAllowed = FALSE;
    };
}