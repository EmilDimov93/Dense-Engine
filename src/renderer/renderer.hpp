// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "abstractions/device.hpp"
#include "abstractions/commandContext.hpp"
#include "abstractions/swapChain.hpp"
#include "abstractions/pipeline.hpp"
#include "abstractions/renderTarget.hpp"

#include "scene/scene.hpp"

#include "common.hpp"

namespace DE
{
    class Renderer
    {
    public:
        Renderer(HWND window, XMUINT2 windowSize);
        ~Renderer();

        void tick();

    private:
        abstractions::Device device;
        abstractions::CommandContext commands;
        abstractions::SwapChain swapChain;
        abstractions::Pipeline pipeline;
        abstractions::RenderTarget renderTarget;

        Scene scene;
    };
}