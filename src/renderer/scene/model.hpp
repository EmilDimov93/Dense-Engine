// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../common.hpp"

#include "mesh.hpp"
#include "material.hpp"

namespace DE
{
    class Model
    {
        std::vector<Mesh> meshes;
        std::vector<Material> materials;
    };
}