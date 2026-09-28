// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../common.hpp"

#include "vertex.hpp"

namespace DE
{
    class Mesh
    {
    public:
        ComPtr<ID3D12Resource> vertexBuffer;
        ComPtr<ID3D12Resource> indexBuffer;
        ComPtr<ID3D12Resource> blasBuffer;
        ComPtr<ID3D12Resource> blasScratchBuffer;
    };
}