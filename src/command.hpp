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
    struct AddModelCommand
    {
        std::string filePath;
    };

    struct AddInstanceCommand
    {
        Handle modelHandle;
    };

    struct LoadSceneCommand
    {
        std::string filePath;
    };

    struct SaveSceneCommand
    {
    };

    using Command = std::variant<AddModelCommand, AddInstanceCommand, LoadSceneCommand, SaveSceneCommand>;
}