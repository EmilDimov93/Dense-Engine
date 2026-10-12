// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include <DirectXMath.h>

namespace DE
{
    struct Rect
    {
        DirectX::XMUINT2 start;
        DirectX::XMUINT2 size;
    };
}