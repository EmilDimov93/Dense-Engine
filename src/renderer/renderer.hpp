// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "abstractions/device.hpp"
#include "abstractions/commandContext.hpp"
#include "abstractions/swapChain.hpp"
#include "abstractions/pipeline.hpp"
#include "abstractions/renderTarget.hpp"

#include "../scene/scene.hpp"

#include "../rect.hpp"

#include "common.hpp"

namespace DE
{
    class Renderer
    {
    public:
        Renderer(HWND window, XMUINT2 windowSize);
        ~Renderer();

        void tick();

        void uploadScene(const SceneDescription &sceneDescription);

        XMUINT2 getSwapChainSize() const;

        void resizeSwapChain(UINT width, UINT height);

        void setViewportRect(Rect rect);

    private:
        abstractions::Device device;
        abstractions::CommandContext commands;
        abstractions::SwapChain swapChain;
        abstractions::Pipeline pipeline;
        abstractions::RenderTarget renderTarget;

        Rect viewportRect;

        Scene scene;
    };
}