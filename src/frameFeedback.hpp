// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include <cstdint>

namespace DE
{
    struct FrameFeedback
    {
        int32_t clickedSceneComponentHandle = -1;
        uint32_t rayCount = 0;
    };
}