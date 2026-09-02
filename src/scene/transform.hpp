// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include <DirectXMath.h>

using namespace DirectX;

namespace DE
{
    struct Transform
    {
        XMFLOAT3 pos;
        XMFLOAT3 rot;
        XMFLOAT3 scale;
    };
}