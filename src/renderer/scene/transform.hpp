// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../common.hpp"

namespace DE
{
    struct Transform
    {
        XMFLOAT3 pos;
        XMFLOAT3 rot;
        XMFLOAT3 scale;

        Transform(XMFLOAT3 pos = {}, XMFLOAT3 rot = {}, XMFLOAT3 scale = {1.f, 1.f, 1.f}) : pos(pos), rot(rot), scale(scale) {}
    };
}