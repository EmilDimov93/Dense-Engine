// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "ui.hpp"

#include "core/assertions.hpp"

#include "fileDialog.hpp"

#include <imgui.h>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_dx12.h>

#include <filesystem>
#include <array>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace DE
{
    UI::UI(HWND windowHandle, ID3D12Device *device, ID3D12CommandQueue *queue, const PerFrame<ComPtr<ID3D12Resource>> &backBuffers) : windowHandle(windowHandle), device(device)
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

    void UI::tick(uint32_t sidebarWidth, SceneDescription &sceneDescription, bool &hasSceneChanged, ClientState &clientState, std::optional<commands::Command> &command)
    {
        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos({0.f, 0.f});
        ImGui::SetNextWindowSize({static_cast<float>(sidebarWidth), ImGui::GetIO().DisplaySize.y});

        ImGui::Begin("Sidebar", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDecoration);

        if (ImGui::Button("Load", ImVec2(0, 0)))
        {
            std::optional<std::filesystem::path> selectedPath = openFileDialog(windowHandle, {L"DE Scene", L"*.descene"});
            if (selectedPath)
                command = commands::LoadSceneCommand(selectedPath->string());
        }

        ImGui::SameLine();

        if (ImGui::Button("Save", ImVec2(0, 0)))
        {
            command = commands::SaveSceneCommand();
        }

        ImGui::SameLine();

        ImGui::BeginChild("FpsBlock", ImVec2(0, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
        ImGui::Text("%.f FPS", ImGui::GetIO().Framerate);
        ImGui::EndChild();

        const float minFov = 0.f;
        const float maxFov = 180.f;
        const char *fovFormat = (clientState.fov < 0.01f) ? "ORTHO" : "%.1f";
        ImGui::SliderScalar("FOV", ImGuiDataType_Float, &clientState.fov, &minFov, &maxFov, fovFormat);

        if (ImGui::Button("Add Model", ImVec2(-FLT_MIN, 0)))
        {
            std::optional<std::filesystem::path> selectedPath = openFileDialog(windowHandle, {L"3D Models", L"*.obj"});
            if (selectedPath)
                command = commands::AddModelCommand(selectedPath->string());
        }

        ImGui::SeparatorText("Model Instances");

        if (ImGui::Button("Add Instance", ImVec2(-FLT_MIN, 0)))
        {
            ImGui::OpenPopup("Models popup");
        }

        if (ImGui::BeginPopup("Models popup"))
        {
            ImGui::BeginTable("Select model", 1, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoHostExtendX | ImGuiTableFlags_Borders);
            ImGui::TableSetupColumn("Model name");
            ImGui::TableHeadersRow();

            for (uint32_t modelIndex = 0; modelIndex < sceneDescription.modelDescriptions.size(); modelIndex++)
            {
                const ModelDescription &modelDescription = sceneDescription.modelDescriptions[modelIndex];
                const std::string fileName = std::filesystem::path(modelDescription.filePath).filename().string();

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::PushID(modelIndex);
                if (ImGui::Selectable(fileName.c_str()))
                {
                    command = commands::AddInstanceCommand(modelDescription.handle);
                }
                ImGui::PopID();
            }

            ImGui::EndTable();
            ImGui::EndPopup();
        }

        auto drawTableXYZHeader = [](const char *name)
        {
            ImGui::BeginTable(name, 4, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Borders);
            ImGui::TableSetupColumn("");
            ImGui::TableSetupColumn("X");
            ImGui::TableSetupColumn("Y");
            ImGui::TableSetupColumn("Z");
            ImGui::TableHeadersRow();
        };

        auto drawTableXYZRow = [&hasSceneChanged](const char *name, float &x, float &y, float &z, float min, float max)
        {
            ImGui::PushID(name);
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(name);

            std::array<float *, 3> components = {&x, &y, &z};
            for (int32_t i = 0; i < 3; i++)
            {
                ImGui::TableNextColumn();
                ImGui::PushID(i);
                ImGui::SetNextItemWidth(50.f);
                hasSceneChanged |= ImGui::DragFloat("##value", components[i], 0.01f, min, max, "%.2f", ImGuiSliderFlags_AlwaysClamp);
                ImGui::PopID();
            }
            ImGui::PopID();
        };

        ImGui::BeginTable("ModelInstancesTable", 2, ImGuiTableFlags_Borders);
        ImGui::TableSetupColumn("Handle", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();
        for (uint32_t rowIndex = 0; rowIndex < sceneDescription.modelInstanceDescriptions.size(); rowIndex++)
        {
            ModelInstanceDescription &modelInstance = sceneDescription.modelInstanceDescriptions[rowIndex];

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::PushID(rowIndex);
            if (ImGui::Selectable(std::to_string(modelInstance.handle).c_str(), clientState.selectedSceneComponentHandle == static_cast<int32_t>(modelInstance.handle), ImGuiSelectableFlags_SpanAllColumns))
            {
                clientState.selectedSceneComponentHandle = static_cast<int32_t>(modelInstance.handle);
                ImGui::OpenPopup("InstanceDetailsPopup");
            }

            if (ImGui::BeginPopup("InstanceDetailsPopup"))
            {
                drawTableXYZHeader("TransformTable");

                drawTableXYZRow("Position", modelInstance.transform.pos.x, modelInstance.transform.pos.y, modelInstance.transform.pos.z, -FLT_MAX, FLT_MAX);
                drawTableXYZRow("Rotation", modelInstance.transform.rot.x, modelInstance.transform.rot.y, modelInstance.transform.rot.z, -FLT_MAX, FLT_MAX);
                drawTableXYZRow("Scale", modelInstance.transform.scale.x, modelInstance.transform.scale.y, modelInstance.transform.scale.z, -FLT_MAX, FLT_MAX);

                ImGui::EndTable();

                ImGui::EndPopup();
            }

            ImGui::PopID();

            ImGui::TableNextColumn();
            for (const ModelDescription &modelDescription : sceneDescription.modelDescriptions)
            {
                if (modelInstance.modelHandle == modelDescription.handle)
                {
                    ImGui::TextUnformatted(std::filesystem::path(modelDescription.filePath).filename().string().c_str());
                    break;
                }
            }
        }
        ImGui::EndTable();

        ImGui::SeparatorText("Lights");

        if (ImGui::Button("Add Light", ImVec2(-FLT_MIN, 0)))
        {
            ImGui::OpenPopup("Lights popup");
        }

        if (ImGui::BeginPopup("Lights popup"))
        {
            ImGui::BeginTable("Select light type", 1, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoHostExtendX | ImGuiTableFlags_Borders);
            ImGui::TableSetupColumn("Light type");
            ImGui::TableHeadersRow();

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::PushID(0);
            if (ImGui::Selectable("Point"))
            {
                command = commands::AddLightCommand(commands::AddLightCommand::LightType::Point);
            }
            ImGui::PopID();

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::PushID(1);
            if (ImGui::Selectable("Directional"))
            {
                command = commands::AddLightCommand(commands::AddLightCommand::LightType::Point);
            }
            ImGui::PopID();

            ImGui::EndTable();
            ImGui::EndPopup();
        }

        const float labelColumnWidth = ImGui::CalcTextSize("Intensity").x + ImGui::GetStyle().ItemSpacing.x;

        auto drawLabeledFloat = [&hasSceneChanged, labelColumnWidth](const char *name, float &value, float min, float max)
        {
            ImGui::PushID(name);
            const float rowStartX = ImGui::GetCursorPosX();
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted(name);
            ImGui::SameLine(rowStartX + labelColumnWidth);
            ImGui::SetNextItemWidth(-FLT_MIN);
            hasSceneChanged |= ImGui::DragFloat("##value", &value, 0.01f, min, max, "%.2f", ImGuiSliderFlags_AlwaysClamp);
            ImGui::PopID();
        };

        ImGui::BeginTable("LightsTable", 2, ImGuiTableFlags_Borders);
        ImGui::TableSetupColumn("Handle", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();
        for (uint32_t rowIndex = 0; rowIndex < sceneDescription.directionalLightDescriptions.size(); rowIndex++)
        {
            DirectionalLightDescription &directionalLight = sceneDescription.directionalLightDescriptions[rowIndex];

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::PushID(rowIndex);
            if (ImGui::Selectable(std::to_string(directionalLight.handle).c_str(), clientState.selectedSceneComponentHandle == static_cast<int32_t>(directionalLight.handle), ImGuiSelectableFlags_SpanAllColumns))
            {
                clientState.selectedSceneComponentHandle = static_cast<int32_t>(directionalLight.handle);
                ImGui::OpenPopup("DirectionalLightDetailsPopup");
            }

            if (ImGui::BeginPopup("DirectionalLightDetailsPopup"))
            {
                drawTableXYZHeader("DirectionAndColorTable");

                drawTableXYZRow("Direction", directionalLight.direction.x, directionalLight.direction.y, directionalLight.direction.z, -FLT_MAX, FLT_MAX);
                drawTableXYZRow("Color", directionalLight.color.x, directionalLight.color.y, directionalLight.color.z, 0.f, 1.f);
                if (hasSceneChanged)
                {
                    const float length = std::sqrt(directionalLight.direction.x * directionalLight.direction.x + directionalLight.direction.y * directionalLight.direction.y + directionalLight.direction.z * directionalLight.direction.z);
                    directionalLight.direction.x /= length;
                    directionalLight.direction.y /= length;
                    directionalLight.direction.z /= length;
                }

                ImGui::EndTable();

                drawLabeledFloat("Intensity", directionalLight.intensity, 0.f, FLT_MAX);

                ImGui::EndPopup();
            }

            ImGui::PopID();

            ImGui::TableNextColumn();
            ImGui::TextUnformatted("Directional");
        }

        for (uint32_t rowIndex = 0; rowIndex < sceneDescription.pointLightDescriptions.size(); rowIndex++)
        {
            PointLightDescription &pointLight = sceneDescription.pointLightDescriptions[rowIndex];

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::PushID(rowIndex);
            if (ImGui::Selectable(std::to_string(pointLight.handle).c_str(), clientState.selectedSceneComponentHandle == static_cast<int32_t>(pointLight.handle), ImGuiSelectableFlags_SpanAllColumns))
            {
                clientState.selectedSceneComponentHandle = static_cast<int32_t>(pointLight.handle);
                ImGui::OpenPopup("PointLightDetailsPopup");
            }

            if (ImGui::BeginPopup("PointLightDetailsPopup"))
            {
                drawTableXYZHeader("PositionAndColorTable");

                drawTableXYZRow("Position", pointLight.pos.x, pointLight.pos.y, pointLight.pos.z, -FLT_MAX, FLT_MAX);
                drawTableXYZRow("Color", pointLight.color.x, pointLight.color.y, pointLight.color.z, 0.f, 1.f);

                ImGui::EndTable();

                drawLabeledFloat("Intensity", pointLight.intensity, 0.f, FLT_MAX);
                drawLabeledFloat("Range", pointLight.range, 0.f, FLT_MAX);

                ImGui::EndPopup();
            }

            ImGui::PopID();

            ImGui::TableNextColumn();
            ImGui::TextUnformatted("Point");
        }
        ImGui::EndTable();

        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered())
            clientState.selectedSceneComponentHandle = -1;

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