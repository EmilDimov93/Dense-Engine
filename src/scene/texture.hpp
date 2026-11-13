// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include <vector>
#include <cstdint>

namespace DE
{
    struct TextureUploadData
    {
        std::vector<uint8_t> pixels;
        uint32_t width, height;
    };
}