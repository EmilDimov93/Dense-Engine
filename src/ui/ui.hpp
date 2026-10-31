// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "core/dxcommon.hpp"

#include "scene/sceneDescription.hpp"

#include "clientState.hpp"
#include "camera.hpp"

#include "command.hpp"
#include "frameFeedback.hpp"

#include "performanceMonitor.hpp"

#include <string>
#include <optional>

namespace DE
{
    class UI
    {
    public:
        UI(HWND windowHandle, ID3D12Device *device, ID3D12CommandQueue *queue, const PerFrame<ID3D12ResourcePtr> &backBuffers);
        ~UI();

        static bool handleMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

        void tick(uint32_t sidebarWidth, float &sidebarWidthFraction, SceneDescription &sceneDescription, bool &hasSceneChanged, ClientState &clientState, std::optional<commands::Command> &command, Camera &camera, FrameFeedback &frameFeedback);
        void draw(ID3D12GraphicsCommandList *list, UINT frameIndex) const;

        void createBackBufferViews(const PerFrame<ID3D12ResourcePtr> &backBuffers);
        D3D12_CPU_DESCRIPTOR_HANDLE backBufferView(UINT frameIndex) const;

    private:
        static constexpr UINT MAX_TEXTURES = 64;

        ID3D12Device *device = nullptr;

        HWND windowHandle = nullptr;

        ID3D12DescriptorHeapPtr textureHeap;
        UINT textureDescriptorSize = 0;
        std::vector<UINT> freeTextureSlots;

        ID3D12DescriptorHeapPtr backBufferViewHeap;
        UINT backBufferViewSize = 0;

        bool isResizingSidebar = false;

        PerformanceMonitor performanceMonitor;

        void updateCamera(Camera &camera);
        void updateShortcuts(std::optional<commands::Command> &command);
        void updateSidebarSize(float &sidebarWidthFraction, uint32_t sidebarWidth);
    };
}