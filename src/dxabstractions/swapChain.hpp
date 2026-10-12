// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "core/dxcommon.hpp"
#include "core/assertions.hpp"

namespace DE::dxabstractions
{
    struct SwapChain
    {
        ComPtr<IDXGISwapChain4> swapchain;
        PerFrame<ComPtr<ID3D12Resource>> backBuffers;
        XMUINT2 size;

        SwapChain(IDXGIFactory6 *factory, ID3D12CommandQueue *graphicsQueue, HWND windowHandle, UINT width, UINT height)
        {
            DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};
            swapchainDesc.Width = width;
            swapchainDesc.Height = height;
            swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            swapchainDesc.Stereo = FALSE;
            swapchainDesc.SampleDesc = {1, 0};
            swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            swapchainDesc.BufferCount = FRAMES_IN_FLIGHT;
            swapchainDesc.Scaling = DXGI_SCALING_NONE;
            swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
            swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
            swapchainDesc.Flags = 0;

            ComPtr<IDXGISwapChain1> swapchain1;
            dxa(factory->CreateSwapChainForHwnd(graphicsQueue, windowHandle, &swapchainDesc, nullptr, nullptr, &swapchain1), "Failed to create swapchain");

            dxa(factory->MakeWindowAssociation(windowHandle, DXGI_MWA_NO_ALT_ENTER), "Failed to make window association");
            dxa(swapchain1.As(&swapchain), "Failed to copy swapchain");

            for (UINT bufferIndex = 0; bufferIndex < FRAMES_IN_FLIGHT; ++bufferIndex)
                dxa(swapchain->GetBuffer(bufferIndex, IID_PPV_ARGS(&backBuffers[bufferIndex])), "Failed to get swapchain buffer");

            size.x = width;
            size.y = height;
        }

        ~SwapChain()
        {
            for (UINT bufferIndex = 0; bufferIndex < FRAMES_IN_FLIGHT; ++bufferIndex)
                backBuffers[bufferIndex].Reset();

            swapchain.Reset();
        }

        void resize(XMUINT2 newSize)
        {
            for (UINT bufferIndex = 0; bufferIndex < FRAMES_IN_FLIGHT; ++bufferIndex)
                backBuffers[bufferIndex].Reset();

            dxa(swapchain->ResizeBuffers(FRAMES_IN_FLIGHT, newSize.x, newSize.y, DXGI_FORMAT_UNKNOWN, 0), "Failed to resize swapchain");

            for (UINT bufferIndex = 0; bufferIndex < FRAMES_IN_FLIGHT; ++bufferIndex)
                dxa(swapchain->GetBuffer(bufferIndex, IID_PPV_ARGS(&backBuffers[bufferIndex])), "Failed to get swapchain buffer");

            size = newSize;
        }
    };
}