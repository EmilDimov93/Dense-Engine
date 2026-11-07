// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "core/handle.hpp"

#include <cstdint>

namespace DE::gpu
{
    struct FrameFeedback
    {
        uint32_t clickedSceneComponentHandle = INVALID_HANDLE;
        uint32_t rayCount = 0;
    };
}