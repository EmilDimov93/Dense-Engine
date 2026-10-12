// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "renderer.hpp"

#include "core/assertions.hpp"

#include <filesystem>
#include <iostream>
#include <source_location>

namespace DE
{
    Renderer::Renderer(HWND window, XMUINT2 windowSize) : commands(device),
                                                          swapChain(device.factory.Get(), commands.queue.Get(), window, windowSize.x, windowSize.y),
                                                          pipeline(device, "shaders/shader.cso"),
                                                          scene(device, commands.list.Get()),
                                                          renderTarget(device, windowSize.x, windowSize.y)
    {
    }

    Renderer::~Renderer()
    {
        waitForGpu();
    }

    void Renderer::waitForGpu()
    {
        for (dxabstractions::Fence &fence : commands.fences)
        {
            fence.signal(commands.queue.Get());
            fence.wait();
        }
    }
}