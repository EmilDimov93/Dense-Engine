// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "core/dxcommon.hpp"
#include "core/assertions.hpp"

namespace DE::dxabstractions
{
    class SwapChain
    {
    public:
        PerFrame<ComPtr<ID3D12Resource>> backBuffers;

        SwapChain(IDXGIFactory6 *factory, ID3D12CommandQueue *graphicsQueue, HWND windowHandle, UINT width, UINT height)
        {
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
            swapChainDesc.Flags = 0;

            ComPtr<IDXGISwapChain1> swapChain1;
            dxa(factory->CreateSwapChainForHwnd(graphicsQueue, windowHandle, &swapChainDesc, nullptr, nullptr, &swapChain1), "Failed to create swap chain");

            dxa(factory->MakeWindowAssociation(windowHandle, DXGI_MWA_NO_ALT_ENTER), "Failed to make window association");
            dxa(swapChain1.As(&swapChain), "Failed to copy swap chain");

            for (UINT bufferIndex = 0; bufferIndex < FRAMES_IN_FLIGHT; ++bufferIndex)
                dxa(swapChain->GetBuffer(bufferIndex, IID_PPV_ARGS(&backBuffers[bufferIndex])), "Failed to get swap chain buffer");

            size.x = width;
            size.y = height;
        }

        ~SwapChain()
        {
            for (UINT bufferIndex = 0; bufferIndex < FRAMES_IN_FLIGHT; ++bufferIndex)
                backBuffers[bufferIndex].Reset();

            swapChain.Reset();
        }

        void present()
        {
            dxa(swapChain->Present(1, 0), "Failed to present");
        }

        void resize(XMUINT2 newSize)
        {
            for (UINT bufferIndex = 0; bufferIndex < FRAMES_IN_FLIGHT; ++bufferIndex)
                backBuffers[bufferIndex].Reset();

            dxa(swapChain->ResizeBuffers(FRAMES_IN_FLIGHT, newSize.x, newSize.y, DXGI_FORMAT_UNKNOWN, 0), "Failed to resize swap chain");

            for (UINT bufferIndex = 0; bufferIndex < FRAMES_IN_FLIGHT; ++bufferIndex)
                dxa(swapChain->GetBuffer(bufferIndex, IID_PPV_ARGS(&backBuffers[bufferIndex])), "Failed to get swap chain buffer");

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
        ComPtr<IDXGISwapChain4> swapChain;
        XMUINT2 size{};
    };
}