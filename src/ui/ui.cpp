// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "ui.hpp"

#include "core/assertions.hpp"

#include <imgui.h>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_dx12.h>

#include <filesystem>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace DE
{
    UI::UI(HWND windowHandle, ID3D12Device *device, ID3D12CommandQueue *queue, const PerFrame<ComPtr<ID3D12Resource>> &backBuffers) : device(device)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();

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
        initInfo.SrvDescriptorHeap = textureHeap.Get();
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

    void UI::tick(uint32_t sidebarWidth, const SceneDescription &sceneDescription, Settings &settings)
    {
        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos({0.f, 0.f});
        ImGui::SetNextWindowSize({static_cast<float>(sidebarWidth), ImGui::GetIO().DisplaySize.y});

        ImGui::Begin("Sidebar", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDecoration);
        
        ImGui::BeginChild("FpsBlock", ImVec2(0, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
        ImGui::Text("%.f FPS", ImGui::GetIO().Framerate);
        ImGui::EndChild();

        const float minFov = 0.f;
        const float maxFov = 180.f;
        const char* fovFormat = (settings.fov < 0.01f) ? "ORTHO" : "%.1f";
        ImGui::SliderScalar("FOV", ImGuiDataType_Float, &settings.fov, &minFov, &maxFov, fovFormat);

        ImGui::SeparatorText("Model Instances");
        ImGui::BeginTable("Table", 3, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoHostExtendX | ImGuiTableFlags_Borders);
        for (uint32_t rowIndex = 0; rowIndex < sceneDescription.modelInstanceDescriptions.size(); ++rowIndex)
        {
            const ModelInstanceDescription &modelInstance = sceneDescription.modelInstanceDescriptions[rowIndex];

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::PushID(rowIndex);
            if (ImGui::Selectable(std::to_string(modelInstance.handle).c_str(), settings.selectedInstanceID == static_cast<int32_t>(rowIndex), ImGuiSelectableFlags_SpanAllColumns))
                settings.selectedInstanceID = rowIndex;
            ImGui::PopID();

            ImGui::TableNextColumn();
            for (const ModelDescription &modelDescription : sceneDescription.modelDescriptions)
            {
                if (modelInstance.modelHandle == modelDescription.handle)
                    ImGui::Text(std::filesystem::path(modelDescription.filePath).filename().string().c_str());
            }
            ImGui::TableNextColumn();
            ImGui::Text("{%.2f, %.2f, %.2f}", modelInstance.transform.pos.x, modelInstance.transform.pos.y, modelInstance.transform.pos.z);
        }
        ImGui::EndTable();

        ImGui::End();

        ImGui::Render();
    }

    void UI::draw(ID3D12GraphicsCommandList *list, UINT frameIndex) const
    {
        const D3D12_CPU_DESCRIPTOR_HANDLE target = backBufferView(frameIndex);
        list->OMSetRenderTargets(1, &target, FALSE, nullptr);

        ID3D12DescriptorHeap *descriptorHeaps[] = {textureHeap.Get()};
        list->SetDescriptorHeaps(static_cast<UINT>(std::size(descriptorHeaps)), descriptorHeaps);

        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), list);
    }

    void UI::createBackBufferViews(const PerFrame<ComPtr<ID3D12Resource>> &backBuffers)
    {
        for (UINT bufferIndex = 0; bufferIndex < FRAMES_IN_FLIGHT; ++bufferIndex)
            device->CreateRenderTargetView(backBuffers[bufferIndex].Get(), nullptr, backBufferView(bufferIndex));
    }

    D3D12_CPU_DESCRIPTOR_HANDLE UI::backBufferView(UINT frameIndex) const
    {
        D3D12_CPU_DESCRIPTOR_HANDLE handle = backBufferViewHeap->GetCPUDescriptorHandleForHeapStart();
        handle.ptr += static_cast<SIZE_T>(frameIndex) * backBufferViewSize;
        return handle;
    }
}