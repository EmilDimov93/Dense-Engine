// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "core/handle.hpp"

#include "transform.hpp"

#include <string>

namespace DE
{
    struct ModelDescription
    {
        Handle handle;
        std::string filePath;
    };

    struct ModelInstanceDescription
    {
        Handle handle;
        Handle modelHandle;
        Transform transform;
    };

    struct SceneDescription
    {
        std::vector<ModelDescription> modelDescriptions;
        std::vector<ModelInstanceDescription> modelInstanceDescriptions;
    };
}