// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "core/dxcommon.hpp"

#include "dxabstractions/accelerationStructure.hpp"

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
        ID3D12ResourcePtr vertexBuffer;
        uint32_t vertexCount;
        ID3D12ResourcePtr indexBuffer;
        uint32_t indexCount;
        
        dxabstractions::AccelerationStructure blas;

        uint32_t materialIndex;

        uint32_t vertexOffset = 0;
        uint32_t indexOffset = 0;

        Mesh(const MeshUploadData &meshUploadData, ID3D12Device5 *device, ID3D12GraphicsCommandList4 *commandList);
    };
}