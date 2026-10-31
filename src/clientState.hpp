// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "core/dxcommon.hpp"

namespace DE
{
    struct ClientState
    {
        int32_t selectedSceneComponentHandle = -1;
        uint32_t ssaaSamples = 4u;
        float time = 0.f;
        uint32_t lowerQuality = 0u;
        XMINT2 clickedPixel = {-1, -1};
    };
}