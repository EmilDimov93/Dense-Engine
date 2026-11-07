// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "ui.hpp"

#include "fileDialog.hpp"

#include <array>
#include <cmath>

#include <imgui.h>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_dx12.h>

namespace DE
{
    void drawTableXYZHeader(const char *name)
    {
        ImGui::BeginTable(name, 4, ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoHostExtendX);
        ImGui::TableSetupColumn("");
        ImGui::TableSetupColumn("X");
        ImGui::TableSetupColumn("Y");
        ImGui::TableSetupColumn("Z");
        ImGui::TableHeadersRow();
    }

    void drawTableXYZRow(const char *name, float &x, float &y, float &z, float min, float max, bool &hasSceneChanged, bool &isDragging)
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
            isDragging |= ImGui::IsItemActive() && ImGui::IsMouseDown(ImGuiMouseButton_Left);
            ImGui::PopID();
        }
        ImGui::PopID();
    }

    std::optional<ImVec2> worldToScreen(const Camera &camera, const XMFLOAT3 &worldPos, ImVec2 viewportMin, ImVec2 viewportSize)
    {
        const gpu::CameraConstants constants = camera.toConstants();

        const XMVECTOR offset = XMLoadFloat3(&worldPos) - XMLoadFloat3(&camera.pos);
        const float x = XMVectorGetX(XMVector3Dot(offset, XMLoadFloat3(&constants.right)));
        const float y = XMVectorGetX(XMVector3Dot(offset, XMLoadFloat3(&constants.up)));
        const float z = XMVectorGetX(XMVector3Dot(offset, XMLoadFloat3(&constants.forward)));

        if (z <= 0.f)
            return std::nullopt;

        const float halfHeight = (camera.fov < 0.01f) ? constants.orthoHalfHeight : z * std::tan(XMConvertToRadians(camera.fov) * 0.5f);
        const float scale = viewportSize.y * 0.5f / halfHeight;

        return ImVec2{viewportMin.x + viewportSize.x * 0.5f + x * scale, viewportMin.y + viewportSize.y * 0.5f - y * scale};
    }

    void UI::tick(SceneDescription &sceneDescription, bool &hasSceneChanged, gpu::ClientState &clientState, std::optional<commands::Command> &command, Camera &camera, const gpu::FrameFeedback &frameFeedback, bool &vsync, scripter::Script &script, bool &isScriptRunning, bool &renderThisFrame)
    {
        bool isDragging = false;

        const ImGuiIO &io = ImGui::GetIO();
        const ImGuiStyle &style = ImGui::GetStyle();

        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos({0.f, 0.f});
        ImGui::SetNextWindowSize({static_cast<float>(sidebarWidthPx), io.DisplaySize.y});

        ImGui::Begin("Sidebar", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBringToFrontOnFocus);

        if (ImGui::Button("Load", ImVec2(0, 0)))
        {
            std::optional<std::filesystem::path> selectedPath = openFileDialog(windowHandle, {L"DE Scene", L"*.descene"});
            if (selectedPath)
                command = commands::LoadScene(selectedPath->string());
        }

        const ImVec2 loadBtnSize = ImGui::GetItemRectSize();

        ImGui::SameLine();

        if (ImGui::Button("Save", ImVec2(0, 0)))
        {
            command = commands::SaveScene();
        }

        const ImVec2 saveBtnSize = ImGui::GetItemRectSize();

        ImGui::SameLine();

        const float topBarRemainingWidth = sidebarWidthPx - loadBtnSize.x - saveBtnSize.x - style.ItemSpacing.x * 4;

        performanceMonitor.draw(topBarRemainingWidth, io.Framerate, frameFeedback.rayCount);

        ImGui::Dummy(ImVec2(0.0f, 8.0f));

        constexpr ImGuiTableFlags scriptTableFlags = ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersInnerH;
        static bool isScripterWindowOpen = false;
        if (ImGui::BeginTable("ScriptTable", 2, scriptTableFlags))
        {
            ImGui::TableSetupColumn("Script", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableHeadersRow();

            ImGui::TableNextRow();

            const bool wasScriptWindowOpen = isScripterWindowOpen;
            if (isScripterWindowOpen)
            {
                ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(225, 22, 28, 255));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(180, 22, 28, 255));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(255, 22, 28, 255));
            }
            ImGui::TableNextColumn();
            if (ImGui::Button(isScripterWindowOpen ? "Close" : "Open", ImVec2(-FLT_MIN, 0)))
            {
                isScripterWindowOpen = !isScripterWindowOpen;
                if (isScripterWindowOpen)
                    sidebarWidthPx = OPTIMAL_SIDEBAR_SCRIPTOR_WIDTH;
            }
            if (wasScriptWindowOpen)
                ImGui::PopStyleColor(3);

            const bool wasScriptRunning = isScriptRunning;
            if (isScriptRunning)
            {
                ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(225, 22, 28, 255));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(180, 22, 28, 255));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(255, 22, 28, 255));
            }
            ImGui::TableNextColumn();
            if (ImGui::Button(isScriptRunning ? "Stop" : "Run", ImVec2(-FLT_MIN, 0)))
                isScriptRunning = !isScriptRunning;
            if (wasScriptRunning)
                ImGui::PopStyleColor(3);

            ImGui::EndTable();
        }

        const float scripterWindowStartY = ImGui::GetCursorPos().y;

        ImGui::Dummy(ImVec2(0.0f, 8.0f));

        const uint32_t ssaaOptions[] = {1, 4, 9, 16};
        ImGui::SetNextItemWidth(50.f);
        if (ImGui::BeginCombo("SSAA", std::format("{}x", clientState.ssaaSamples).c_str()))
        {
            for (uint32_t option : ssaaOptions)
                if (ImGui::Selectable(std::format("{}x", option).c_str(), clientState.ssaaSamples == option))
                    clientState.ssaaSamples = option;
            ImGui::EndCombo();
        }

        ImGui::SameLine();
        ImGui::Checkbox("VSync", &vsync);

        const float maxSliderWidth = std::min(ImGui::GetContentRegionAvail().x, 130.f);

        ImGui::Dummy(ImVec2(0.0f, 8.0f));

        ImGui::SetNextItemWidth(maxSliderWidth);
        const char *fovFormat = (camera.fov < 0.01f) ? "ORTHO" : "%.f";
        ImGui::SliderScalar("FOV", ImGuiDataType_Float, &camera.fov, &MIN_FOV, &MAX_FOV, fovFormat);

        ImGui::Dummy(ImVec2(0.0f, 8.0f));

        ImGui::SetNextItemWidth(maxSliderWidth);
        ImGui::SliderScalar("Move Speed", ImGuiDataType_Float, &camera.moveSpeed, &Camera::MIN_MOVE_SPEED, &Camera::MAX_MOVE_SPEED, "%.1f", ImGuiSliderFlags_Logarithmic);

        ImGui::Dummy(ImVec2(0.0f, 8.0f));

        drawTableXYZHeader("Background");

        drawTableXYZRow("Background", sceneDescription.clearColor.x, sceneDescription.clearColor.y, sceneDescription.clearColor.z, MIN_COLOR, MAX_COLOR, hasSceneChanged, isDragging);

        ImGui::EndTable();

        ImGui::Dummy(ImVec2(0.0f, 8.0f));

        ImGui::SetNextItemWidth(maxSliderWidth);
        hasSceneChanged |= ImGui::SliderScalar("Exposure", ImGuiDataType_Float, &sceneDescription.exposure, &MIN_EXPOSURE, &MAX_EXPOSURE, "%.2f", ImGuiSliderFlags_Logarithmic);

        ImGui::Dummy(ImVec2(0.0f, 8.0f));

        if (ImGui::Button("Add Model", ImVec2(-FLT_MIN, 0)))
        {
            std::optional<std::filesystem::path> selectedPath = openFileDialog(windowHandle, {L"3D Models", L"*.obj"});
            if (selectedPath)
                command = commands::AddModel(selectedPath->string());
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
                    command = commands::AddInstance(modelDescription.handle);
                }
                ImGui::PopID();
            }

            ImGui::EndTable();
            ImGui::EndPopup();
        }

        const float tableRowHeight = ImGui::GetTextLineHeight() + style.CellPadding.y * 2.f;

        float instancesTableHeight = tableRowHeight * static_cast<float>(std::min(static_cast<uint32_t>(sceneDescription.modelInstanceDescriptions.size()), 10u) + 1) + 2.f;
        if (ImGui::BeginTable("ModelInstancesTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY, ImVec2(0.f, instancesTableHeight)))
        {
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

                    drawTableXYZRow("Position", modelInstance.transform.pos.x, modelInstance.transform.pos.y, modelInstance.transform.pos.z, -FLT_MAX, FLT_MAX, hasSceneChanged, isDragging);
                    drawTableXYZRow("Rotation", modelInstance.transform.rot.x, modelInstance.transform.rot.y, modelInstance.transform.rot.z, -FLT_MAX, FLT_MAX, hasSceneChanged, isDragging);
                    drawTableXYZRow("Scale", modelInstance.transform.scale.x, modelInstance.transform.scale.y, modelInstance.transform.scale.z, -FLT_MAX, FLT_MAX, hasSceneChanged, isDragging);

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
        }

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
                command = commands::AddLight(commands::AddLight::LightType::Point);
            }
            ImGui::PopID();

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::PushID(1);
            if (ImGui::Selectable("Directional"))
            {
                command = commands::AddLight(commands::AddLight::LightType::Directional);
            }
            ImGui::PopID();

            ImGui::EndTable();
            ImGui::EndPopup();
        }

        const float labelColumnWidth = ImGui::CalcTextSize("Intensity").x + style.ItemSpacing.x;

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

        float lightsTableHeight = tableRowHeight * static_cast<float>(std::min(static_cast<uint32_t>(sceneDescription.pointLightDescriptions.size() + sceneDescription.directionalLightDescriptions.size() + 1), 10u) + 1) + 2.f;
        if (ImGui::BeginTable("LightsTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY, ImVec2(0.f, lightsTableHeight)))
        {
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
                    drawTableXYZRow("Color", sceneDescription.ambient.x, sceneDescription.ambient.y, sceneDescription.ambient.z, MIN_COLOR, MAX_COLOR, hasSceneChanged, isDragging);
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

                    drawTableXYZRow("Direction", directionalLight.direction.x, directionalLight.direction.y, directionalLight.direction.z, -FLT_MAX, FLT_MAX, hasSceneChanged, isDragging);
                    drawTableXYZRow("Color", directionalLight.color.x, directionalLight.color.y, directionalLight.color.z, MIN_COLOR, MAX_COLOR, hasSceneChanged, isDragging);
                    if (hasSceneChanged)
                    {
                        const float length = std::sqrt(directionalLight.direction.x * directionalLight.direction.x + directionalLight.direction.y * directionalLight.direction.y + directionalLight.direction.z * directionalLight.direction.z);
                        if (length > 0.f)
                        {
                            directionalLight.direction.x /= length;
                            directionalLight.direction.y /= length;
                            directionalLight.direction.z /= length;
                        }
                    }

                    ImGui::EndTable();

                    drawLabeledFloat("Intensity", directionalLight.intensity, MIN_INTENSITY, MAX_INTENSITY);

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

                    drawTableXYZRow("Position", pointLight.pos.x, pointLight.pos.y, pointLight.pos.z, -FLT_MAX, FLT_MAX, hasSceneChanged, isDragging);
                    drawTableXYZRow("Color", pointLight.color.x, pointLight.color.y, pointLight.color.z, MIN_COLOR, MAX_COLOR, hasSceneChanged, isDragging);

                    ImGui::EndTable();

                    drawLabeledFloat("Intensity", pointLight.intensity, MIN_INTENSITY, MAX_INTENSITY);
                    drawLabeledFloat("Range", pointLight.range, MIN_RANGE, MAX_RANGE);

                    ImGui::EndPopup();
                }

                if (clientState.selectedSceneComponentHandle == static_cast<int32_t>(pointLight.handle))
                {
                    const ImVec2 displaySize = io.DisplaySize;
                    const ImVec2 viewportMin{static_cast<float>(sidebarWidthPx), 0.f};
                    const ImVec2 viewportSize{displaySize.x - viewportMin.x, displaySize.y};

                    if (const auto lightScreenPos = worldToScreen(camera, pointLight.pos, viewportMin, viewportSize))
                    {
                        const ImVec4 markerColor{pointLight.color.x, pointLight.color.y, pointLight.color.z, 0.8f};
                        ImGui::GetBackgroundDrawList()->AddCircleFilled(*lightScreenPos, 5.f, ImGui::ColorConvertFloat4ToU32(markerColor));
                    }
                }

                ImGui::PopID();

                ImGui::TableNextColumn();
                ImGui::TextUnformatted("point");
            }
            ImGui::EndTable();
        }

        const bool hasCameraChanged = updateCamera(camera);
        updateShortcuts(command);
        const bool hasSidebarResized = updateSidebarSize();

        if (hasSidebarResized)
            command = commands::ResizeSidebar(sidebarWidthPx);

        if (isScripterWindowOpen)
        {
            Rect scripterWindowRect;
            scripterWindowRect.start.x = 0;
            scripterWindowRect.start.y = static_cast<uint32_t>(scripterWindowStartY);
            scripterWindowRect.size.x = sidebarWidthPx;
            scripterWindowRect.size.y = static_cast<uint32_t>(io.DisplaySize.y - scripterWindowStartY);
            drawScripterWindow(script, scripterWindowRect);
        }

        ImGui::End();

        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            clientState.selectedSceneComponentHandle = -1;
            if (!io.WantCaptureMouse && ImGui::GetMousePos().x > sidebarWidthPx)
                clientState.clickedPixel = XMINT2{static_cast<int32_t>(ImGui::GetMousePos().x - sidebarWidthPx), static_cast<int32_t>(ImGui::GetMousePos().y)};
        }
        else
        {
            clientState.clickedPixel = XMINT2{-1, -1};
        }

        if (frameFeedback.clickedSceneComponentHandle != -1)
            clientState.selectedSceneComponentHandle = frameFeedback.clickedSceneComponentHandle;

        ImGui::Render();

        clientState.lowerQuality = isDragging || hasCameraChanged;

        renderThisFrame = hasCameraChanged || ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseReleased(ImGuiMouseButton_Left);
    }

    void UI::draw(ID3D12GraphicsCommandList *list, D3D12_CPU_DESCRIPTOR_HANDLE target) const
    {
        list->OMSetRenderTargets(1, &target, FALSE, nullptr);

        ID3D12DescriptorHeap *descriptorHeaps[] = {textureHeap};
        list->SetDescriptorHeaps(static_cast<UINT>(std::size(descriptorHeaps)), descriptorHeaps);

        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), list);
    }

    bool UI::updateCamera(Camera &camera)
    {
        const ImGuiIO &io = ImGui::GetIO();
        if (io.WantTextInput || io.KeyCtrl)
            return false;

        constexpr float fovStepDegPerWheelNotch = 5.f;
        bool isZooming = false;
        if (!io.WantCaptureMouse && io.MouseWheel != 0.f)
        {
            camera.fov = std::clamp(camera.fov - io.MouseWheel * fovStepDegPerWheelNotch, MIN_FOV, MAX_FOV);
            isZooming = true;
        }

        const float dolly = static_cast<float>(ImGui::IsKeyDown(ImGuiKey_W) - ImGui::IsKeyDown(ImGuiKey_S));
        const float truck = static_cast<float>(ImGui::IsKeyDown(ImGuiKey_D) - ImGui::IsKeyDown(ImGuiKey_A));
        float pan = 0.f;
        float tilt = 0.f;
        if (!io.WantCaptureMouse && ImGui::IsMouseDown(ImGuiMouseButton_Right))
        {
            pan = io.MouseDelta.x;
            tilt = io.MouseDelta.y;
        }

        const bool hasCameraChanged = dolly != 0.f || truck != 0.f || pan != 0.f || tilt != 0.f || isZooming;

        const float dt = io.DeltaTime;

        camera.rot.y += pan * camera.lookSensitivityRadPerPixel;

        camera.rot.x += tilt * camera.lookSensitivityRadPerPixel;
        camera.rot.x = std::clamp(camera.rot.x, -XM_PI / 2.f, XM_PI / 2.f);

        if (dolly == 0.f && truck == 0.f)
            return hasCameraChanged;

        const gpu::CameraConstants constants = camera.toConstants();
        const XMVECTOR forward = XMLoadFloat3(&constants.forward);
        const XMVECTOR right = XMLoadFloat3(&constants.right);

        const XMVECTOR direction = XMVector3Normalize(forward * dolly + right * truck);
        const XMVECTOR pos = XMLoadFloat3(&camera.pos) + direction * (camera.moveSpeed * dt);

        XMStoreFloat3(&camera.pos, pos);

        return hasCameraChanged;
    }

    void UI::updateShortcuts(std::optional<commands::Command> &command)
    {
        if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsKeyPressed(ImGuiKey_S))
            command = commands::SaveScene();

        if (ImGui::IsKeyPressed(ImGuiKey_F2))
            command = commands::Screenshot();
    }

    bool UI::updateSidebarSize()
    {
        if (ImGui::IsAnyItemActive())
            return false;

        const ImGuiIO &io = ImGui::GetIO();

        float cursorX = io.MousePos.x;

        const bool isHoveringEdge = std::abs(cursorX - static_cast<float>(sidebarWidthPx)) < 5.f;

        if (isHoveringEdge && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            isResizingSidebar = true;
        if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
            isResizingSidebar = false;

        if (isHoveringEdge || isResizingSidebar)
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);

        if (isResizingSidebar)
        {
            const uint32_t oldSidebarWidthPx = sidebarWidthPx;
            sidebarWidthPx = static_cast<uint32_t>(std::clamp(cursorX, 1.f, 0.7f * io.DisplaySize.x));
            if (std::abs(static_cast<int32_t>(cursorX) - static_cast<int32_t>(OPTIMAL_SIDEBAR_WIDTH)) <= 10)
                sidebarWidthPx = OPTIMAL_SIDEBAR_WIDTH;

            if (sidebarWidthPx != oldSidebarWidthPx)
                return true;
        }

        return false;
    }
}