// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

/*
    DLOD (Dynamic Level Of Detail) adjusts the SSAA samples per pixel, downscaling or 
    ray recursion depth to sustain a stable framerate.
*/

#pragma once

#include <chrono>
#include <algorithm>
#include <cstdint>

namespace DE
{
    class DLOD
    {
    public:
        void adjustLOD(uint32_t &ssaaSamples, uint32_t &downscaleFactor, uint32_t &lowerQuality, bool decreaseQuality, bool increaseQuality)
        {
            const auto currentTime = std::chrono::steady_clock::now();

            const float fps = 1.f / std::chrono::duration<float>(currentTime - prevFrameTime).count();

            prevFrameTime = currentTime;

            if (decreaseQuality)
                LOD = MAX_SSAA - 1;

            if (adjustmentDelay == 0)
            {
                if (fps < MIN_TARGET_FPS)
                    LOD++;
                else if (fps > MAX_TARGET_FPS && LOD != MIN_LOD)
                    LOD--;

                adjustmentDelay = FRAMES_PER_ADJUSTMENT;
            }

            adjustmentDelay--;

            if (increaseQuality)
                LOD = MIN_LOD;

            LOD = std::clamp(LOD, MIN_LOD, MAX_LOD);

            if (LOD < MAX_SSAA)
            {
                downscaleFactor = 1;
                ssaaSamples = MAX_SSAA - LOD;
            }
            else
            {
                downscaleFactor = LOD - MAX_SSAA + 2;
                ssaaSamples = 1;
                if (LOD == MAX_LOD)
                    lowerQuality = true;
            }
        }

    private:
        uint32_t LOD = 0;
        std::chrono::steady_clock::time_point prevFrameTime = std::chrono::steady_clock::now();

        uint32_t adjustmentDelay = 0;

        static constexpr float MIN_TARGET_FPS = 55.f;
        static constexpr float MAX_TARGET_FPS = 100.f;

        static constexpr uint32_t MAX_SSAA = 32u;
        static constexpr uint32_t MAX_DOWNSCALE = 4u;

        static constexpr uint32_t MIN_LOD = 0u;
        static constexpr uint32_t MAX_LOD = MAX_SSAA + MAX_DOWNSCALE - 2u;

        static constexpr uint32_t FRAMES_PER_ADJUSTMENT = 3u;
    };
}