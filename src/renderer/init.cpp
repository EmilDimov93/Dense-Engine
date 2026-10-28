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