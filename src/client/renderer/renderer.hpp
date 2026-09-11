// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "abstractions/device.hpp"
#include "abstractions/commandContext.hpp"
#include "abstractions/swapChain.hpp"

#include "common.hpp"

namespace DE
{
    class Renderer
    {
    public:
        Renderer(HWND window, XMUINT2 windowSize);
        ~Renderer();

    private:
        abstractions::Device device;
        abstractions::CommandContext commands;
        abstractions::SwapChain swapChain;
    };
}