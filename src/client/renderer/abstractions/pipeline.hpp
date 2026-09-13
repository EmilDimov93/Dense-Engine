// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../assertions.hpp"
#include "../common.hpp"

namespace DE::abstractions
{
    struct Pipeline
    {
        ComPtr<ID3D12RootSignature> globalRootSignature;
        ComPtr<ID3D12StateObject> stateObject;
        ComPtr<ID3D12Resource> shaderTable;
    };
}