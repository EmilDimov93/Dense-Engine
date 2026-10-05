// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../common.hpp"

#include "mesh.hpp"
#include "material.hpp"

namespace DE
{
    struct ModelUploadData
    {
        std::vector<MeshUploadData> meshes;
        std::vector<MaterialUploadData> materials;
    };

    class Model
    {
    public:
        std::vector<Mesh> meshes;
        std::vector<Material> materials;
    };
}