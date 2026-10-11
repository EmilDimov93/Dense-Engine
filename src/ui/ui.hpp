// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include <windows.h>

#include <DirectXMath.h>

namespace DE
{
    class UI
    {
    public:
    private:
        DirectX::XMUINT2 viewportStart;
        DirectX::XMUINT2 viewportSize;

        DirectX::XMUINT2 windowSize;
    };
}