// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <comdef.h>

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

    inline constexpr uint32_t FRAMES_IN_FLIGHT = 2;

    template <typename T>
    using PerFrame = std::array<T, FRAMES_IN_FLIGHT>;

#define MAKE_SMART_COM_POINTER(_a) _COM_SMARTPTR_TYPEDEF(_a, __uuidof(_a))

    MAKE_SMART_COM_POINTER(IDXGIFactory6);
    MAKE_SMART_COM_POINTER(IDXGIAdapter1);
    MAKE_SMART_COM_POINTER(IDXGISwapChain4);
    MAKE_SMART_COM_POINTER(ID3D12Device5);
    MAKE_SMART_COM_POINTER(ID3D12CommandQueue);
    MAKE_SMART_COM_POINTER(ID3D12CommandAllocator);
    MAKE_SMART_COM_POINTER(ID3D12GraphicsCommandList4);
    MAKE_SMART_COM_POINTER(ID3D12Resource);
    MAKE_SMART_COM_POINTER(ID3D12DescriptorHeap);
    MAKE_SMART_COM_POINTER(ID3D12Fence);
    MAKE_SMART_COM_POINTER(ID3D12Debug);
    MAKE_SMART_COM_POINTER(IDXGISwapChain1);
    MAKE_SMART_COM_POINTER(ID3D12RootSignature);
    MAKE_SMART_COM_POINTER(ID3D12StateObject);
    MAKE_SMART_COM_POINTER(ID3D12StateObjectProperties);
    MAKE_SMART_COM_POINTER(ID3DBlob);
}