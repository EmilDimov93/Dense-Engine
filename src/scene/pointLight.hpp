// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "core/dxcommon.hpp"

namespace DE
{
    struct PointLight
    {
        XMFLOAT3 pos;
        float intensity;
        XMFLOAT3 color;
        float range;
    };
}