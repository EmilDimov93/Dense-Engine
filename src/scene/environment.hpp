// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "core/dxcommon.hpp"

namespace DE::gpu
{
    struct Environment
    {
        XMFLOAT3 clearColor = {0.f, 0.f, 0.f};
        float exposure = 1.f;
        XMFLOAT3 ambient = {0.2f, 0.2f, 0.2f};
    };
}