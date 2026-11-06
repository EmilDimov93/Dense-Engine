// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "instructions.hpp"

#include "scene/sceneDescription.hpp"

#include <vector>

namespace DE::scripter
{
    inline const char *INFO_TEXT = "Write scripts that modify the scene every frame.\nEvery instruction adds its value to a property of its scene component.\n\nScene components:\n MI (Model Instance)\n PL (Point Light) \n DL (Directional Light)\n\nInstructions:\n MOVE - moves MI or PL\n ROTATE - rotates MI\n SCALE - scales MI\n DIRECTION - changes dir of DL\n COLOR - adds to color of PL or DL\n INTENSITY - adds to intensity of PL or DL\n RANGE - adds to range of PL\n EXPOSURE - adds to scene exposure\n AMBIENT - adds to scene ambient light";

    using Script = std::vector<scripter::instruction::Instruction>;

    bool execute(const Script &script, SceneDescription &sceneDescription);
}