// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../common.hpp"

namespace DE
{
    struct Material
    {
        XMFLOAT4 diffuse{1.f, 1.f, 1.f, 1.f};
        float metallic{0};
        float roughness{1};

        std::vector<uint8_t> texture;
        XMUINT2 textureSize{0, 0};

        bool hasTexture()
        {
            return !texture.empty();
        }
    };
}