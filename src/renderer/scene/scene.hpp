// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../common.hpp"

#include "model.hpp"
#include "modelInstance.hpp"

#include <unordered_map>
#include <string>

namespace DE
{
    class Scene
    {
    public:
        Handle addModel(const std::string &filePath);
        Handle addModelInstance(Handle modelHandle, Transform transform);
    private:
        std::unordered_map<Handle, Model> models;
        std::unordered_map<Handle, ModelInstance> modelInstances;
        
        HandleGenerator handleGenerator;
    };
}