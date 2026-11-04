// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "ui.hpp"

#include "core/assertions.hpp"

#include "fonts/jetBrainsMonoNLExtraBold.hpp"

#include <imgui.h>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_dx12.h>

#include <filesystem>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace DE
{
    UI::UI(HWND windowHandle, ID3D12Device *device, ID3D12CommandQueue *queue, const PerFrame<ID3D12ResourcePtr> &backBuffers) : windowHandle(windowHandle), device(device)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();

        const ImVec4 green = ImVec4(0.24f, 0.68f, 0.42f, 1.00f);
        const ImVec4 greenStrong = ImVec4(0.12f, 0.60f, 0.33f, 1.00f);
        const ImVec4 greenDark = ImVec4(0.15f, 0.40f, 0.26f, 1.00f);
        const ImVec4 greenGrab = ImVec4(0.22f, 0.60f, 0.37f, 1.00f);

        auto withAlpha = [](ImVec4 c, float a)
        { c.w = a; return c; };

        ImGuiStyle &style = ImGui::GetStyle();
        ImVec4 *colors = style.Colors;

        colors[ImGuiCol_Button] = withAlpha(green, 0.12f);
        colors[ImGuiCol_ButtonHovered] = withAlpha(green, 0.35f);
        colors[ImGuiCol_ButtonActive] = withAlpha(greenStrong, 0.55f);
        colors[ImGuiCol_FrameBgHovered] = withAlpha(green, 0.15f);
        colors[ImGuiCol_FrameBgActive] = withAlpha(green, 0.30f);
        colors[ImGuiCol_SliderGrab] = withAlpha(greenGrab, 0.70f);
        colors[ImGuiCol_SliderGrabActive] = green;
        colors[ImGuiCol_CheckMark] = green;
        colors[ImGuiCol_Header] = withAlpha(green, 0.10f);
        colors[ImGuiCol_HeaderHovered] = withAlpha(green, 0.28f);
        colors[ImGuiCol_HeaderActive] = withAlpha(green, 0.45f);
        colors[ImGuiCol_TitleBgActive] = withAlpha(greenDark, 0.50f);
        colors[ImGuiCol_Tab] = withAlpha(greenDark, 0.30f);
        colors[ImGuiCol_TabHovered] = withAlpha(green, 0.35f);
        colors[ImGuiCol_TextSelectedBg] = withAlpha(green, 0.20f);
        colors[ImGuiCol_ResizeGrip] = withAlpha(green, 0.08f);
        colors[ImGuiCol_ResizeGripHovered] = withAlpha(green, 0.28f);
        colors[ImGuiCol_ResizeGripActive] = withAlpha(green, 0.45f);

        style.ScrollbarSize = 6.f;
        style.ScrollbarRounding = 3.f;

        ImGui_ImplWin32_Init(windowHandle);

        D3D12_DESCRIPTOR_HEAP_DESC textureHeapDesc{};
        textureHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        textureHeapDesc.NumDescriptors = MAX_TEXTURES;
        textureHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        dxa(device->CreateDescriptorHeap(&textureHeapDesc, IID_PPV_ARGS(&textureHeap)), "Failed to create ui texture heap");
        textureDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

        for (UINT slot = MAX_TEXTURES; slot > 0; --slot)
            freeTextureSlots.push_back(slot - 1);

        D3D12_DESCRIPTOR_HEAP_DESC backBufferViewHeapDesc{};
        backBufferViewHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        backBufferViewHeapDesc.NumDescriptors = FRAMES_IN_FLIGHT;
        dxa(device->CreateDescriptorHeap(&backBufferViewHeapDesc, IID_PPV_ARGS(&backBufferViewHeap)), "Failed to create ui back buffer view heap");
        backBufferViewSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        createBackBufferViews(backBuffers);

        ImGui_ImplDX12_InitInfo initInfo;
        initInfo.Device = device;
        initInfo.CommandQueue = queue;
        initInfo.NumFramesInFlight = FRAMES_IN_FLIGHT;
        initInfo.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
        initInfo.DSVFormat = DXGI_FORMAT_UNKNOWN;
        initInfo.UserData = this;
        initInfo.SrvDescriptorHeap = textureHeap;
        initInfo.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo *info, D3D12_CPU_DESCRIPTOR_HANDLE *cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE *gpuHandle)
        {
            UI *ui = static_cast<UI *>(info->UserData);
            if (ui->freeTextureSlots.empty())
                fatal("Out of ui texture descriptors");

            const UINT slot = ui->freeTextureSlots.back();
            ui->freeTextureSlots.pop_back();

            cpuHandle->ptr = ui->textureHeap->GetCPUDescriptorHandleForHeapStart().ptr + static_cast<SIZE_T>(slot) * ui->textureDescriptorSize;
            gpuHandle->ptr = ui->textureHeap->GetGPUDescriptorHandleForHeapStart().ptr + static_cast<UINT64>(slot) * ui->textureDescriptorSize;
        };
        initInfo.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo *info, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE)
        {
            UI *ui = static_cast<UI *>(info->UserData);
            const SIZE_T offset = cpuHandle.ptr - ui->textureHeap->GetCPUDescriptorHandleForHeapStart().ptr;
            ui->freeTextureSlots.push_back(static_cast<UINT>(offset / ui->textureDescriptorSize));
        };
        ImGui_ImplDX12_Init(&initInfo);

        ImFont *font = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(JetBrainsMonoNLExtraBold_compressed_data, JetBrainsMonoNLExtraBold_compressed_size, 14.0f);
        if (font != nullptr)
            ImGui::GetIO().FontDefault = font;
    }

    UI::~UI()
    {
        ImGui_ImplDX12_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }

    bool UI::handleMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        return ImGui_ImplWin32_WndProcHandler(hwnd, message, wParam, lParam) != 0;
    }

    void UI::createBackBufferViews(const PerFrame<ID3D12ResourcePtr> &backBuffers)
    {
        for (UINT bufferIndex = 0; bufferIndex < FRAMES_IN_FLIGHT; ++bufferIndex)
            device->CreateRenderTargetView(backBuffers[bufferIndex], nullptr, backBufferView(bufferIndex));
    }

    D3D12_CPU_DESCRIPTOR_HANDLE UI::backBufferView(UINT frameIndex) const
    {
        D3D12_CPU_DESCRIPTOR_HANDLE handle = backBufferViewHeap->GetCPUDescriptorHandleForHeapStart();
        handle.ptr += static_cast<SIZE_T>(frameIndex) * backBufferViewSize;
        return handle;
    }
}