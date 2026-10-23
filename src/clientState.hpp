// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include <cstdint>

namespace DE
{
    struct ClientState
    {
        int32_t selectedSceneComponentHandle = -1;
        uint32_t ssaaSamples = 4u;
        float time = 0.f;
    };
}