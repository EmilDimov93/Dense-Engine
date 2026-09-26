// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../common.hpp"

#include "model.hpp"
#include "modelInstance.hpp"

namespace DE
{
    class Scene
    {
    public:
    private:
        std::vector<Model> models;
        std::vector<ModelInstance> modelInstances;
    };
}