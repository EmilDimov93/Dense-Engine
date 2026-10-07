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
        ID3D12CommandAllocator *uploadCommandAllocator = commands.allocators[0].Get();
        dxa(uploadCommandAllocator->Reset(), "Failed to reset command allocator");
        dxa(commands.list->Reset(uploadCommandAllocator, nullptr), "Failed to reset command list");

        Handle cube = scene.addModel("../../example/assets/cube.obj");
        scene.addModelInstance(cube, Transform({0.f, 0.f, 3.f}, {0.5f, 0.5f, 0.f}));

        scene.upload();

        dxa(commands.list->Close(), "Failed to close command list");

        ID3D12CommandList *commandListsToExecute[] = {commands.list.Get()};
        commands.queue->ExecuteCommandLists(static_cast<UINT>(std::size(commandListsToExecute)), commandListsToExecute);

        commands.fences[0].signal(commands.queue.Get());
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