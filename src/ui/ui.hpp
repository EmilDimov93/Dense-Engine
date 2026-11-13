// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "core/dxcommon.hpp"
#include "core/rect.hpp"

#include "scene/sceneDescription.hpp"

#include "clientState.hpp"
#include "camera.hpp"

#include "command.hpp"
#include "frameFeedback.hpp"

#include "scripter/scripter.hpp"

#include "performanceMonitor.hpp"

#include <string>
#include <optional>

namespace DE
{
    class UI
    {
    public:
        UI(HWND windowHandle, ID3D12Device *device, ID3D12CommandQueue *queue);
        ~UI();

        static bool handleMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

        void tick(SceneDescription &sceneDescription, bool &hasSceneChanged, gpu::ClientState &clientState, std::optional<commands::Command> &command, Camera &camera, const gpu::FrameFeedback &frameFeedback, bool &vsync, scripter::Script &script, bool &isScriptRunning, bool &onDemandRendering, bool &dynamicLOD, bool &isInteracting);
        void draw(ID3D12GraphicsCommandList *list, D3D12_CPU_DESCRIPTOR_HANDLE target) const;

        uint32_t getSidebarWidthPx() const { return sidebarWidthPx; }

    private:
        static constexpr UINT MAX_TEXTURES = 64;

        static constexpr float MIN_FOV = 0.f;
        static constexpr float MAX_FOV = 179.f;

        static constexpr uint32_t OPTIMAL_SIDEBAR_WIDTH = 215;
        static constexpr uint32_t OPTIMAL_SIDEBAR_SCRIPTER_WIDTH = 314;

        ID3D12Device *device = nullptr;

        HWND windowHandle = nullptr;

        ID3D12DescriptorHeapPtr textureHeap;
        UINT textureDescriptorSize = 0;
        std::vector<UINT> freeTextureSlots;

        PerformanceMonitor performanceMonitor;

        uint32_t sidebarWidthPx = OPTIMAL_SIDEBAR_WIDTH;

        bool isResizingSidebar = false;

        bool isScripterWindowOpen = false;
        bool isBackgroundImageTabOpen = false;
        
        bool updateCamera(Camera &camera);
        bool updateSidebarSize();
        void updateShortcuts(std::optional<commands::Command> &command);
        void drawScripterWindow(scripter::Script &script, Rect windowRect);
    };
}