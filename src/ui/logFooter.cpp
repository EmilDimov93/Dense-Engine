// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "ui.hpp"

namespace DE
{
    void drawLogLevelLabel(log::Level level)
    {
        const char *label = "";
        ImVec4 color{1.f, 1.f, 1.f, 1.f};

        switch (level)
        {
        case log::Level::Info:
            label = "Info";
            color = ImVec4(0.40f, 0.70f, 1.00f, 1.f);
            break;
        case log::Level::Warning:
            label = "Warning";
            color = ImVec4(1.00f, 0.80f, 0.00f, 1.f);
            break;
        case log::Level::Fatal:
            label = "Fatal";
            color = ImVec4(0.88f, 0.09f, 0.11f, 1.f);
            break;
        }

        ImGui::TextColored(color, "%s", label);
    }

    float UI::drawLogFooter()
    {
        log::getEntries(log::MAX_ENTRIES, logEntries);

        constexpr size_t TOOLTIP_ENTRY_COUNT = 20;
        const float width = static_cast<float>(sidebarWidthPx);

        auto drawEntry = [](const log::Entry &entry)
        {
            drawLogLevelLabel(entry.getLevel());
            ImGui::SameLine(0.f, 0.f);
            ImGui::Text(": %s", entry.getMsg().c_str());
        };

        ImGui::SetNextWindowPos({0.f, ImGui::GetIO().DisplaySize.y}, ImGuiCond_Always, {0.f, 1.f});
        ImGui::SetNextWindowSizeConstraints({width, 0.f}, {width, FLT_MAX});

        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4{0.1f, 0.1f, 0.1f, 1.f});
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {ImGui::GetStyle().WindowPadding.x, 4.f});
        ImGui::Begin("LogFooter", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav);
        ImGui::PopStyleVar();

        ImGui::SeparatorText(("Log (" + std::to_string(logEntries.size()) + ")").c_str());

        const ImVec2 rowPos = ImGui::GetCursorPos();
        ImGui::InvisibleButton("##LogHover", {-FLT_MIN, ImGui::GetFrameHeight()});
        const bool isHovered = ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip);
        ImGui::SetCursorPos(rowPos);

        ImGui::AlignTextToFramePadding();
        if (!logEntries.empty())
            drawEntry(logEntries.back());

        if (isHovered && !logEntries.empty() && ImGui::BeginTooltip())
        {
            const size_t firstEntry = logEntries.size() - std::min(logEntries.size(), TOOLTIP_ENTRY_COUNT);
            for (size_t i = firstEntry; i < logEntries.size(); i++)
                drawEntry(logEntries[i]);

            ImGui::EndTooltip();
        }

        const float footerHeight = ImGui::GetWindowHeight();

        ImGui::End();
        ImGui::PopStyleColor(1);

        return footerHeight;
    }
}