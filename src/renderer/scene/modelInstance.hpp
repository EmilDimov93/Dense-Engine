// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../common.hpp"

#include "model.hpp"
#include "transform.hpp"

using Handle = uint32_t;

namespace DE
{
    class ModelInstance
    {
        Handle modelHandle;
        Transform transform;
    };
}