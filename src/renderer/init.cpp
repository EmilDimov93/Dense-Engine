// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "renderer.hpp"

#include "core/assertions.hpp"

namespace DE
{
    Renderer::Renderer(HWND window, XMUINT2 windowSize) : commands(device),
                                                          swapChain(device.factory.Get(), commands.queue.Get(), window, windowSize.x, windowSize.y),
                                                          pipeline(device),
                                                          renderTarget(device, windowSize.x, windowSize.y),
                                                          scene(device, commands.list.Get())
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