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
    class UI;

    class Renderer
    {
    public:
        Renderer(HWND window, XMUINT2 windowSize);
        ~Renderer();

        void tick(UI &ui);

        void waitForGpu();

        void uploadScene(const SceneDescription &sceneDescription);

        XMUINT2 getSwapChainSize() const;

        void resizeSwapChain(XMUINT2 size);

        void setViewportRect(Rect rect);

        ID3D12Device5 *getDevice() const { return device; }
        ID3D12CommandQueue *getQueue() const { return commands.queue.Get(); }
        const PerFrame<ComPtr<ID3D12Resource>> &getBackBuffers() const { return swapChain.backBuffers; }

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