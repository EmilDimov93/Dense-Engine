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
        static constexpr const char *NAME = "MOVE";
        Handle handle = INVALID_HANDLE;
        XMFLOAT3 pos{};
    };

    struct ROTATE
    {
        static constexpr const char *NAME = "ROTATE";
        Handle handle = INVALID_HANDLE;
        XMFLOAT3 rot{};
    };

    struct SCALE
    {
        static constexpr const char *NAME = "SCALE";
        Handle handle = INVALID_HANDLE;
        XMFLOAT3 scale{};
    };

    struct COLOR
    {
        static constexpr const char *NAME = "COLOR";
        Handle handle = INVALID_HANDLE;
        XMFLOAT3 color{};
    };

    struct INTENSITY
    {
        static constexpr const char *NAME = "INTENSITY";
        Handle handle = INVALID_HANDLE;
        float intensity = 0.f;
    };

    struct RANGE
    {
        static constexpr const char *NAME = "RANGE";
        Handle handle = INVALID_HANDLE;
        float range = 0.f;
    };

    struct EXPOSURE
    {
        static constexpr const char *NAME = "EXPOSURE";
        float exposure = 0.f;
    };

    struct AMBIENT
    {
        static constexpr const char *NAME = "AMBIENT";
        XMFLOAT3 ambient{};
    };

    using Instruction = std::variant<MOVE, ROTATE, SCALE, COLOR, INTENSITY, RANGE, EXPOSURE, AMBIENT>;
}