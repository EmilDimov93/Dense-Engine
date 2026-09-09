// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "abstractions/device.hpp"
#include "abstractions/commandContext.hpp"

#include "common.hpp"

namespace DE
{
    class Renderer
    {
    public:
        Renderer();
        ~Renderer();

    private:
        Device device;
        CommandContext commands;
    };
}