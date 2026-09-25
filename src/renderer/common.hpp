// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

#include <DirectXMath.h>
using namespace DirectX;

#include <cstdint>
#include <array>
#include <vector>

namespace DE
{
#ifdef _DEBUG
    inline constexpr bool DEBUG_MODE = true;
#else
    inline constexpr bool DEBUG_MODE = false;
#endif

    template <typename T>
    using ComPtr = Microsoft::WRL::ComPtr<T>;

    static constexpr uint32_t FRAMES_IN_FLIGHT = 2;

    template <typename T>
    using PerFrame = std::array<T, FRAMES_IN_FLIGHT>;
}