// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "model.hpp"

#include <string>

namespace DE
{
    [[nodiscard]] ModelUploadData loadModel(const std::string &filePath);
}