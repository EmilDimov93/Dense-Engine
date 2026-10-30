// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "core/dxcommon.hpp"

namespace DE
{
    struct Transform
    {
        XMFLOAT3 pos;
        XMFLOAT3 rot;
        XMFLOAT3 scale;

        Transform(XMFLOAT3 pos = {}, XMFLOAT3 rot = {}, XMFLOAT3 scale = {1.f, 1.f, 1.f}) : pos(pos), rot(rot), scale(scale) {}

        bool operator!=(const Transform &other) const { return pos.x != other.pos.x || pos.y != other.pos.y || pos.z != other.pos.z || rot.x != other.rot.x || rot.y != other.rot.y || rot.z != other.rot.z || scale.x != other.scale.x || scale.y != other.scale.y || scale.z != other.scale.z; }
    };
}