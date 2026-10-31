// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "renderer.hpp"

#include "core/assertions.hpp"

namespace DE
{
    Renderer::Renderer(HWND window, XMUINT2 windowSize) : commands(device),
                                                          swapChain(device.factory, commands.queue, window, windowSize.x, windowSize.y),
                                                          pipeline(device),
                                                          descriptorHeap(device),
                                                          renderTarget(device, descriptorHeap.cpuHandle(dxabstractions::DESCRIPTOR_HEAP_OUTPUT_UAV_INDEX), windowSize.x, windowSize.y),
                                                          scene(device, commands.list, &descriptorHeap)
    {
        frameFeedbackBuffer = createBuffer(device, sizeof(FrameFeedback), D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

        for (ID3D12ResourcePtr &readback : frameFeedbackReadbackBuffers)
            readback = createBuffer(device, sizeof(FrameFeedback), D3D12_HEAP_TYPE_READBACK, D3D12_RESOURCE_STATE_COPY_DEST);
    }

    Renderer::~Renderer()
    {
        waitForGpu();
    }

    void Renderer::waitForGpu()
    {
        commands.flush();
    }
}