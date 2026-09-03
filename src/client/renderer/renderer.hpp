// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "device.hpp"

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
    };
}