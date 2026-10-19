// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "core/dxcommon.hpp"

#include "scene/sceneDescription.hpp"

#include "settings.hpp"

namespace DE
{
    class UI
    {
    public:
        UI(HWND windowHandle, ID3D12Device *device, ID3D12CommandQueue *queue, const PerFrame<ComPtr<ID3D12Resource>> &backBuffers);
        ~UI();

        static bool handleMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

        void tick(uint32_t sidebarWidth, SceneDescription &sceneDescription, bool &hasSceneChanged, Settings &settings);
        void draw(ID3D12GraphicsCommandList *list, UINT frameIndex) const;

        void createBackBufferViews(const PerFrame<ComPtr<ID3D12Resource>> &backBuffers);
        D3D12_CPU_DESCRIPTOR_HANDLE backBufferView(UINT frameIndex) const;

    private:
        static constexpr UINT MAX_TEXTURES = 64;

        ID3D12Device *device = nullptr;

        ComPtr<ID3D12DescriptorHeap> textureHeap;
        UINT textureDescriptorSize = 0;
        std::vector<UINT> freeTextureSlots;

        ComPtr<ID3D12DescriptorHeap> backBufferViewHeap;
        UINT backBufferViewSize = 0;
    };
}