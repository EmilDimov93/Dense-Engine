// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "ui.hpp"

#include "core/assertions.hpp"

#include "fileDialog.hpp"
#include "fonts/jetBrainsMonoNLExtraBold.hpp"

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

        const ImVec4 green = ImVec4(0.24f, 0.68f, 0.42f, 1.00f);
        const ImVec4 greenStrong = ImVec4(0.12f, 0.60f, 0.33f, 1.00f);
        const ImVec4 greenDark = ImVec4(0.15f, 0.40f, 0.26f, 1.00f);
        const ImVec4 greenGrab = ImVec4(0.22f, 0.60f, 0.37f, 1.00f);

        auto withAlpha = [](ImVec4 c, float a)
        { c.w = a; return c; };

        ImVec4 *colors = ImGui::GetStyle().Colors;

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

    void drawTableXYZHeader(const char *name)
    {
        ImGui::BeginTable(name, 4, ImGuiTableFlags_Borders);
        ImGui::TableSetupColumn("");
        ImGui::TableSetupColumn("X");
        ImGui::TableSetupColumn("Y");
        ImGui::TableSetupColumn("Z");
        ImGui::TableHeadersRow();
    }

    void drawTableXYZRow(const char *name, float &x, float &y, float &z, float min, float max, bool &hasSceneChanged)
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
            ImGui::SetNextItemWidth(32.f);
            hasSceneChanged |= ImGui::DragFloat("##value", components[i], 0.01f, min, max, "%.2f", ImGuiSliderFlags_AlwaysClamp);
            ImGui::PopID();
        }
        ImGui::PopID();
    }

    void UI::tick(uint32_t sidebarWidth, SceneDescription &sceneDescription, bool &hasSceneChanged, ClientState &clientState, std::optional<commands::Command> &command, Camera &camera)
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

        const float fpsBlockWidth = ImGui::CalcTextSize("FPS: 0000").x + ImGui::GetStyle().FramePadding.x * 2.0f;
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - fpsBlockWidth);
        ImGui::BeginChild("FpsBlock", ImVec2(fpsBlockWidth, 0), ImGuiChildFlags_FrameStyle | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
        ImGui::Text("FPS: %.f", ImGui::GetIO().Framerate);
        ImGui::EndChild();

        ImGui::Dummy(ImVec2(0.0f, 8.0f));

        const float minFov = 0.f;
        const float maxFov = 180.f;
        const char *fovFormat = (camera.fov < 0.01f) ? "ORTHO" : "%.f";
        ImGui::SliderScalar("FOV", ImGuiDataType_Float, &camera.fov, &minFov, &maxFov, fovFormat);

        ImGui::Dummy(ImVec2(0.0f, 8.0f));

        const float minCameraSpeed = 0.f;
        const float maxCameraSpeed = 100.f;
        ImGui::SliderScalar("Camera Speed", ImGuiDataType_Float, &camera.moveSpeed, &minCameraSpeed, &maxCameraSpeed, "%.1f", ImGuiSliderFlags_Logarithmic);

        ImGui::Dummy(ImVec2(0.0f, 8.0f));

        drawTableXYZHeader("Background");

        drawTableXYZRow("Background", sceneDescription.clearColor.x, sceneDescription.clearColor.y, sceneDescription.clearColor.z, 0.f, 1.f, hasSceneChanged);

        ImGui::EndTable();

        ImGui::Dummy(ImVec2(0.0f, 8.0f));

        const float minExposure = 0.f;
        const float maxExposure = 100.f;
        hasSceneChanged |= ImGui::SliderScalar("Exposure", ImGuiDataType_Float, &sceneDescription.exposure, &minExposure, &maxExposure, "%.2f", ImGuiSliderFlags_Logarithmic);

        ImGui::Dummy(ImVec2(0.0f, 8.0f));

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

                drawTableXYZRow("Position", modelInstance.transform.pos.x, modelInstance.transform.pos.y, modelInstance.transform.pos.z, -FLT_MAX, FLT_MAX, hasSceneChanged);
                drawTableXYZRow("Rotation", modelInstance.transform.rot.x, modelInstance.transform.rot.y, modelInstance.transform.rot.z, -FLT_MAX, FLT_MAX, hasSceneChanged);
                drawTableXYZRow("Scale", modelInstance.transform.scale.x, modelInstance.transform.scale.y, modelInstance.transform.scale.z, -FLT_MAX, FLT_MAX, hasSceneChanged);

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

        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::PushID("Ambient");
            const bool isAmbientPopupOpen = ImGui::IsPopupOpen("AmbientLightDetailsPopup");
            if (ImGui::Selectable("-", isAmbientPopupOpen, ImGuiSelectableFlags_SpanAllColumns))
                ImGui::OpenPopup("AmbientLightDetailsPopup");

            if (ImGui::BeginPopup("AmbientLightDetailsPopup"))
            {
                drawTableXYZHeader("ColorTable");
                drawTableXYZRow("Color", sceneDescription.ambient.x, sceneDescription.ambient.y, sceneDescription.ambient.z, 0.f, 1.f, hasSceneChanged);
                ImGui::EndTable();

                ImGui::EndPopup();
            }

            ImGui::PopID();

            ImGui::TableNextColumn();
            ImGui::TextUnformatted("ambient");
        }

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

                drawTableXYZRow("Direction", directionalLight.direction.x, directionalLight.direction.y, directionalLight.direction.z, -FLT_MAX, FLT_MAX, hasSceneChanged);
                drawTableXYZRow("Color", directionalLight.color.x, directionalLight.color.y, directionalLight.color.z, 0.f, 1.f, hasSceneChanged);
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
            ImGui::TextUnformatted("directional");
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

                drawTableXYZRow("Position", pointLight.pos.x, pointLight.pos.y, pointLight.pos.z, -FLT_MAX, FLT_MAX, hasSceneChanged);
                drawTableXYZRow("Color", pointLight.color.x, pointLight.color.y, pointLight.color.z, 0.f, 1.f, hasSceneChanged);

                ImGui::EndTable();

                drawLabeledFloat("Intensity", pointLight.intensity, 0.f, FLT_MAX);
                drawLabeledFloat("Range", pointLight.range, 0.f, FLT_MAX);

                ImGui::EndPopup();
            }

            ImGui::PopID();

            ImGui::TableNextColumn();
            ImGui::TextUnformatted("point");
        }
        ImGui::EndTable();

        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered())
            clientState.selectedSceneComponentHandle = -1;

        updateCamera(camera);
        updateShortcuts(command);

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

    void UI::updateCamera(Camera &camera)
    {
        const ImGuiIO &io = ImGui::GetIO();
        if (io.WantTextInput || io.KeyCtrl)
            return;

        const float dolly = static_cast<float>(ImGui::IsKeyDown(ImGuiKey_W) - ImGui::IsKeyDown(ImGuiKey_S));
        const float truck = static_cast<float>(ImGui::IsKeyDown(ImGuiKey_D) - ImGui::IsKeyDown(ImGuiKey_A));
        float pan = 0.f;
        float tilt = 0.f;
        if (!io.WantCaptureMouse && ImGui::IsMouseDown(ImGuiMouseButton_Right))
        {
            pan = io.MouseDelta.x;
            tilt = io.MouseDelta.y;
        }

        const float dt = io.DeltaTime;

        camera.rot.y += pan * camera.lookSensitivityRadPerPixel;

        camera.rot.x += tilt * camera.lookSensitivityRadPerPixel;
        camera.rot.x = std::clamp(camera.rot.x, -XM_PI / 2.f, XM_PI / 2.f);

        if (dolly == 0.f && truck == 0.f)
            return;

        const CameraConstants constants = camera.toConstants();
        const XMVECTOR forward = XMLoadFloat3(&constants.forward);
        const XMVECTOR right = XMLoadFloat3(&constants.right);

        const XMVECTOR direction = XMVector3Normalize(forward * dolly + right * truck);
        const XMVECTOR pos = XMLoadFloat3(&camera.pos) + direction * (camera.moveSpeed * dt);

        XMStoreFloat3(&camera.pos, pos);
    }

    void UI::updateShortcuts(std::optional<commands::Command> &command)
    {
        if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsKeyPressed(ImGuiKey_S))
            command = commands::SaveSceneCommand();
    }
}