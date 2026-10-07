// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "renderer.hpp"

#include "assertions.hpp"
#include <filesystem>
#include <iostream>
#include <source_location>
namespace DE
{
    Renderer::Renderer(HWND window, XMUINT2 windowSize) : commands(device), swapChain(device.factory.Get(), commands.queue.Get(), window, windowSize.x, windowSize.y), pipeline(device, "shaders/shader.cso"), scene(device, commands.list.Get()), renderTarget(device, windowSize.x, windowSize.y)
    {
        commands.begin(0);

        Handle cube = scene.addModel("../../example/assets/cube.obj");
        scene.addModelInstance(cube, Transform({0.f, 0.f, 3.f}, {0.5f, 0.5f, 0.f}));

        scene.upload();

        commands.execute(0);
        commands.fences[0].wait();
    }

    Renderer::~Renderer()
    {
        for (abstractions::Fence &fence : commands.fences)
        {
            fence.signal(commands.queue.Get());
            fence.wait();
        }
    }
}