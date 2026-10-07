// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "handle.hpp"

#include "scene/transform.hpp"

#include <string>

namespace DE
{
    struct ModelDescription
    {
        const Handle handle;
        const std::string filePath;
    };

    struct ModelInstanceDescription
    {
        const Handle handle;
        const Handle modelHandle;
        Transform transform;
    };

    struct SceneDescription
    {
        std::vector<ModelDescription> modelDescriptions;
        std::vector<ModelInstanceDescription> modelInstanceDescriptions;
    };
}