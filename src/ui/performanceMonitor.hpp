// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include <cstdint>
#include <array>

#include <imgui.h>

namespace DE
{
    struct PerformanceMonitor
    {
        static constexpr uint32_t historySize = 5000;
        std::array<float, historySize> fpsHistory{};
        std::array<float, historySize> rayCountHistory{};
        uint32_t historyOffset = 0;

        float fpsMax = 0.f;
        float rayCountMax = 0.f;

        void draw(float width, float currentFps, uint32_t currentRayCount)
        {
            fpsHistory[historyOffset] = currentFps;
            rayCountHistory[historyOffset] = static_cast<float>(currentRayCount) / 1'000'000.f;

            fpsMax = std::max(fpsMax, fpsHistory[historyOffset]);
            rayCountMax = std::max(rayCountMax, rayCountHistory[historyOffset]);

            historyOffset = (historyOffset + 1) % historySize;

            ImVec2 plotSize(width, 20);
            ImVec2 plotCursorPos = ImGui::GetCursorPos();

            ImGui::PushStyleColor(ImGuiCol_PlotLines, IM_COL32(0, 255, 0, 255));
            ImGui::PlotLines("##FpsPlot", fpsHistory.data(), historySize, historyOffset, nullptr, 0.0f, fpsMax, plotSize);
            ImGui::PopStyleColor(1);

            ImGui::SameLine();

            ImGui::SetCursorPos(plotCursorPos);
            ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_PlotLines, IM_COL32(208, 0, 255, 255));
            ImGui::PlotLines("##RayPlot", rayCountHistory.data(), historySize, historyOffset, nullptr, 0.0f, rayCountMax, plotSize);
            ImGui::PopStyleColor(2);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("FPS\n"
                                  "  Current  %7.1f\n"
                                  "  Peak     %7.1f\n"
                                  "\n"
                                  "Rays per frame\n"
                                  "  Current  %7.2f M\n"
                                  "  Peak     %7.2f M",
                                  fpsHistory[historyOffset - 1], fpsMax, rayCountHistory[historyOffset - 1], rayCountMax);
        }
    };
}