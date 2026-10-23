// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "dxabstractions/device.hpp"
#include "dxabstractions/commandContext.hpp"
#include "dxabstractions/swapChain.hpp"
#include "dxabstractions/pipeline.hpp"
#include "dxabstractions/renderTarget.hpp"

#include "scene/scene.hpp"

#include "core/rect.hpp"
#include "core/dxcommon.hpp"

#include "clientState.hpp"
#include "camera.hpp"

namespace DE
{
    class UI;

    class Renderer
    {
    public:
        Renderer(HWND window, XMUINT2 windowSize);
        ~Renderer();

        void tick(const UI &ui, const ClientState &clientState, const CameraConstants &cameraConstants);

        void waitForGpu();

        void loadScene(const SceneDescription &sceneDescription);
        void uploadScene(const SceneDescription &sceneDescription);

        XMUINT2 getSwapChainSize() const;

        void resizeSwapChain(XMUINT2 size);

        void setViewportRect(Rect rect);

        ID3D12Device5 *getDevice() const { return device; }
        ID3D12CommandQueue *getQueue() const { return commands.queue.Get(); }
        const PerFrame<ComPtr<ID3D12Resource>> &getBackBuffers() const { return swapChain.backBuffers; }

    private:
        dxabstractions::Device device;
        dxabstractions::CommandContext commands;
        dxabstractions::SwapChain swapChain;
        dxabstractions::Pipeline pipeline;
        dxabstractions::RenderTarget renderTarget;

        Rect viewportRect;

        Scene scene;
    };
}