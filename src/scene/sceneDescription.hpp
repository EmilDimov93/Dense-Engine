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

    struct PointLightDescription
    {
        Handle handle;
        XMFLOAT3 pos;
        float intensity;
        XMFLOAT3 color;
        float range;
    };

    struct DirectionalLightDescription
    {
        Handle handle;
        XMFLOAT3 direction;
        float intensity;
        XMFLOAT3 color;
    };

    struct SceneDescription
    {
        std::vector<ModelDescription> modelDescriptions;
        std::vector<ModelInstanceDescription> modelInstanceDescriptions;
        std::vector<PointLightDescription> pointLightDescriptions;
        std::vector<DirectionalLightDescription> directionalLightDescriptions;
    };
}