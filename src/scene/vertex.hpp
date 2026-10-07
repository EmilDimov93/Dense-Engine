// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../renderer/common.hpp"

namespace DE
{
    struct Vertex
    {
        XMFLOAT3 pos;
        XMFLOAT3 normal;
        XMFLOAT2 tex;

        constexpr Vertex() = default;
        constexpr Vertex(XMFLOAT3 pos, XMFLOAT3 normal, XMFLOAT2 tex = {}) : pos(pos), normal(normal), tex(tex) {}
    };
}