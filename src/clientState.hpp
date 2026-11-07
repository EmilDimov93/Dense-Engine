// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "core/dxcommon.hpp"
#include "core/handle.hpp"

namespace DE::gpu
{
    struct ClientState
    {
        uint32_t selectedSceneComponentHandle = INVALID_HANDLE;
        uint32_t ssaaSamples = 4u;
        float time = 0.f;
        uint32_t lowerQuality = 0u;
        XMINT2 clickedPixel = {-1, -1};
    };
}