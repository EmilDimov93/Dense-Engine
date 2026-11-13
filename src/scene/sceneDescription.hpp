// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "core/handle.hpp"

#include "transform.hpp"

#include <string>

namespace DE
{
    inline constexpr float MIN_INTENSITY = 0.f;
    inline constexpr float MAX_INTENSITY = FLT_MAX;
    inline constexpr float MIN_RANGE = 0.f;
    inline constexpr float MAX_RANGE = FLT_MAX;
    inline constexpr float MIN_COLOR = 0.f;
    inline constexpr float MAX_COLOR = 1.f;
    inline constexpr float MIN_EXPOSURE = 0.f;
    inline constexpr float MAX_EXPOSURE = 100.f;

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

        std::string backgroundTextureFilePath;

        XMFLOAT3 clearColor = {0.f, 0.f, 0.f};
        float exposure = 1.f;
        XMFLOAT3 ambient = {0.2f, 0.2f, 0.2f};
    };
}