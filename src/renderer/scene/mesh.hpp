// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include <d3d12.h>
#include <wrl/client.h>

#include <vector>

#include "vertex.hpp"

namespace DE
{
    struct MeshUploadData
    {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        uint32_t materialIndex;
    };

    class Mesh
    {
    public:
        Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer;
        Microsoft::WRL::ComPtr<ID3D12Resource> indexBuffer;
        Microsoft::WRL::ComPtr<ID3D12Resource> blasBuffer;
        Microsoft::WRL::ComPtr<ID3D12Resource> blasScratchBuffer;
    };
}