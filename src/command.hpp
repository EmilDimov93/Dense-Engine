// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

/*
    Commands exchanged between Client and UI containing 
    information about additions to the scene.
*/

#pragma once

#include "core/handle.hpp"

#include <string>
#include <variant>

namespace DE::commands
{
    struct AddModel
    {
        std::string filePath;
    };

    struct AddInstance
    {
        Handle modelHandle;
    };

    struct AddLight
    {
        enum LightType
        {
            Point,
            Directional
        } lightType;
    };

    struct LoadScene
    {
        std::string filePath;
    };

    struct SaveScene
    {
    };

    struct ResizeSidebar
    {
        uint32_t sidebarWidthPx;
    };

    struct Screenshot
    {
    };

    using Command = std::variant<AddModel, AddInstance, AddLight, LoadScene, SaveScene, ResizeSidebar, Screenshot>;
}