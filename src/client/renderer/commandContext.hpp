// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "assertions.hpp"
#include "common.hpp"

namespace DE
{
    struct CommandContext
    {
        ComPtr<ID3D12CommandQueue> queue = nullptr;
        ComPtr<ID3D12CommandAllocator> allocators = nullptr;
        ComPtr<ID3D12GraphicsCommandList4> list = nullptr;

        CommandContext()
        {
        }

        ~CommandContext()
        {
        }
    };
}