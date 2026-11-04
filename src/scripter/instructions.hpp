// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "core/handle.hpp"
#include "core/dxcommon.hpp"

#include <variant>

namespace DE::scripter::instruction
{
    struct MOVE
    {
        Handle handle = INVALID_HANDLE;
        XMFLOAT3 pos{};
    };

    struct ROTATE
    {
        Handle handle = INVALID_HANDLE;
        XMFLOAT3 rot{};
    };

    struct SCALE
    {
        Handle handle = INVALID_HANDLE;
        XMFLOAT3 scale{};
    };

    struct COLOR
    {
        Handle handle = INVALID_HANDLE;
        XMFLOAT3 color{};
    };

    struct INTENSITY
    {
        Handle handle = INVALID_HANDLE;
        float intensity = 0.f;
    };

    struct RANGE
    {
        Handle handle = INVALID_HANDLE;
        float range = 0.f;
    };

    struct EXPOSURE
    {
        float exposure = 0.f;
    };

    struct AMBIENT
    {
        XMFLOAT3 ambient{};
    };

    using Instruction = std::variant<MOVE, ROTATE, SCALE, COLOR, INTENSITY, RANGE, EXPOSURE, AMBIENT>;
}