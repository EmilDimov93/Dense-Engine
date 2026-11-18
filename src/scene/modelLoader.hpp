// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "model.hpp"
#include "texture.hpp"

#include <string>
#include <optional>

namespace DE
{
    [[nodiscard]] std::optional<ModelUploadData> loadModel(const std::string &filePath);

    [[nodiscard]] std::optional<TextureUploadData> loadTexture(const std::string &filePath);
}