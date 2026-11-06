// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "ui.hpp"

namespace DE
{
    void UI::drawScripterWindow(scripter::Script &script, Rect windowRect)
    {
        using namespace scripter::instruction;

        ImGui::SetNextWindowSize({static_cast<float>(windowRect.size.x), static_cast<float>(windowRect.size.y)});
        ImGui::SetNextWindowPos({static_cast<float>(windowRect.start.x), static_cast<float>(windowRect.start.y)});

        if (ImGui::Begin("Script", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
        {
            if (ImGui::Button("Add", ImVec2(40.f, 0.f)))
            {
                ImGui::OpenPopup("Instructions popup");
            }

            if (ImGui::BeginPopup("Instructions popup"))
            {
                ImGui::BeginTable("Select instruction", 1, ImGuiTableFlags_Borders);
                ImGui::TableSetupColumn("Op", ImGuiTableColumnFlags_WidthFixed, 100.f);
                ImGui::TableHeadersRow();

                static const Instruction ALL_INSTRUCTIONS[] = {
                    MOVE{}, ROTATE{}, SCALE{}, DIRECTION{}, COLOR{}, INTENSITY{}, RANGE{}, EXPOSURE{}, AMBIENT{}};

                auto getName = [](const auto &i)
                {
                    return std::decay_t<decltype(i)>::NAME;
                };

                for (const Instruction &instruction : ALL_INSTRUCTIONS)
                    if (ImGui::Selectable(std::visit(getName, instruction)))
                        script.push_back(instruction);

                ImGui::EndTable();
                ImGui::EndPopup();
            }

            ImGui::SameLine();

            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + std::max(0.f, ImGui::GetContentRegionAvail().x - 50.f));

            ImGui::Button("Help", ImVec2(50.f, 0.f));

            if (ImGui::BeginItemTooltip())
            {
                ImGui::TextUnformatted(scripter::INFO_TEXT);
                ImGui::EndTooltip();
            }

            int32_t removeIndex = -1;
            const float tableValueColumnWidth = (ImGui::CalcTextSize("00.00").x + ImGui::GetStyle().FramePadding.x * 2.0f) * 3.0f + ImGui::GetStyle().ItemInnerSpacing.x * 2.0f;
            const float tableWidth = ImGui::CalcTextSize("00").x + ImGui::CalcTextSize("INTENSITY").x + ImGui::CalcTextSize("Handle").x + tableValueColumnWidth + 16.f + 5 * ImGui::GetStyle().CellPadding.x * 2.0f + (5 + 1) * 1.0f;
            if (ImGui::BeginTable("##InstructionsTable", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_NoHostExtendX, ImVec2(tableWidth, 0.f)))
            {
                ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed, ImGui::CalcTextSize("00").x);
                ImGui::TableSetupColumn("Op", ImGuiTableColumnFlags_WidthFixed, ImGui::CalcTextSize("INTENSITY").x);
                ImGui::TableSetupColumn("Handle", ImGuiTableColumnFlags_WidthFixed, ImGui::CalcTextSize("Handle").x);
                ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthFixed, tableValueColumnWidth);
                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize, 16.f);
                ImGui::TableHeadersRow();

                auto drawInstructionRow = [](Instruction &instruction)
                {
                    const char *name = "";
                    Handle *handle = nullptr;
                    float *value = nullptr;
                    int32_t componentCount = 3;
                    float speed = 0.001f;

                    if (MOVE *move = std::get_if<MOVE>(&instruction))
                    {
                        name = MOVE::NAME;
                        handle = &move->handle;
                        value = &move->pos.x;
                        speed = 0.0005f;
                    }
                    else if (ROTATE *rotate = std::get_if<ROTATE>(&instruction))
                    {
                        name = ROTATE::NAME;
                        handle = &rotate->handle;
                        value = &rotate->rot.x;
                    }
                    else if (SCALE *scale = std::get_if<SCALE>(&instruction))
                    {
                        name = SCALE::NAME;
                        handle = &scale->handle;
                        value = &scale->scale.x;
                        speed = 0.0005f;
                    }
                    else if (DIRECTION *direction = std::get_if<DIRECTION>(&instruction))
                    {
                        name = DIRECTION::NAME;
                        handle = &direction->handle;
                        value = &direction->dir.x;
                        speed = 0.0005f;
                    }
                    else if (COLOR *color = std::get_if<COLOR>(&instruction))
                    {
                        name = COLOR::NAME;
                        handle = &color->handle;
                        value = &color->color.x;
                    }
                    else if (INTENSITY *intensity = std::get_if<INTENSITY>(&instruction))
                    {
                        name = INTENSITY::NAME;
                        handle = &intensity->handle;
                        value = &intensity->intensity;
                        componentCount = 1;
                        speed = 0.01f;
                    }
                    else if (RANGE *range = std::get_if<RANGE>(&instruction))
                    {
                        name = RANGE::NAME;
                        handle = &range->handle;
                        value = &range->range;
                        componentCount = 1;
                        speed = 0.01f;
                    }
                    else if (EXPOSURE *exposure = std::get_if<EXPOSURE>(&instruction))
                    {
                        name = EXPOSURE::NAME;
                        value = &exposure->exposure;
                        componentCount = 1;
                        speed = 0.01f;
                    }
                    else if (AMBIENT *ambient = std::get_if<AMBIENT>(&instruction))
                    {
                        name = AMBIENT::NAME;
                        value = &ambient->ambient.x;
                    }

                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted(name);

                    ImGui::TableNextColumn();
                    if (handle)
                    {
                        ImGui::SetNextItemWidth(-FLT_MIN);
                        ImGui::InputScalar("##handle", ImGuiDataType_U32, handle);
                    }
                    else
                    {
                        ImGui::TextDisabled("-");
                    }

                    ImGui::TableNextColumn();
                    ImGui::SetNextItemWidth(-FLT_MIN);
                    ImGui::DragScalarN("##value", ImGuiDataType_Float, value, componentCount, speed, nullptr, nullptr, "%.2f");
                };

                for (uint32_t i = 0; i < script.size(); i++)
                {
                    ImGui::PushID(i);
                    ImGui::TableNextRow();

                    ImGui::TableNextColumn();
                    ImGui::Text("%d", i);

                    drawInstructionRow(script[i]);

                    ImGui::TableNextColumn();
                    if (ImGui::SmallButton("X"))
                        removeIndex = static_cast<int32_t>(i);

                    ImGui::PopID();
                }

                ImGui::EndTable();
            }

            if (removeIndex != -1)
                script.erase(script.begin() + removeIndex);
        }
        ImGui::End();
    }
}