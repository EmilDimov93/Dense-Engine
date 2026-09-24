// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include <vector>

#include "vertex.hpp"

namespace DE
{
    class Mesh
    {
        public:
            std::vector<Vertex> vertices;
            std::vector<uint32_t> indices;

            uint32_t materialIndex;
        private:
    };
}