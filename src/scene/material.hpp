// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../renderer/common.hpp"

namespace DE
{
    struct MaterialUploadData
    {
        XMFLOAT4 diffuse{1.f, 1.f, 1.f, 1.f};
        float metallic{0};
        float roughness{1};

        std::vector<uint8_t> texture;
        XMUINT2 textureSize{0, 0};

        [[nodiscard]] bool hasTexture() const
        {
            return !texture.empty();
        }
    };

    struct Material
    {
        XMFLOAT4 diffuse;
        float metallic;
        float roughness;
        int32_t textureIndex;
    };
}