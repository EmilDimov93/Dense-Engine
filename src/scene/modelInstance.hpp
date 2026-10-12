// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "core/dxcommon.hpp"
#include "core/handle.hpp"

#include "model.hpp"
#include "transform.hpp"

namespace DE
{
    class ModelInstance
    {
    public:
        Handle modelHandle;
        Transform transform;

        ModelInstance(Handle modelHandle, Transform transform) : modelHandle(modelHandle), transform(transform) {}
    };
}