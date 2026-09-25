// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../common.hpp"

#include "mesh.hpp"

namespace DE
{
    struct Material
    {
        XMFLOAT4 diffuse;
        float metallic;
        float roughness;

        std::vector<uint8_t> texture;
    };
}