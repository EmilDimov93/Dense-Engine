// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "mesh.hpp"

#include "dxabstractions/buffer.hpp"

#include "core/assertions.hpp"

namespace DE
{
    Mesh::Mesh(const MeshUploadData &meshUploadData, ID3D12Device5 *device, ID3D12GraphicsCommandList4 *commandList) : vertexCount(static_cast<uint32_t>(meshUploadData.vertices.size())),
                                                                                                                       indexCount(static_cast<uint32_t>(meshUploadData.indices.size())),
                                                                                                                       materialIndex(meshUploadData.materialIndex)
    {
        if (vertexCount == 0 || indexCount == 0 || indexCount % 3 != 0)
            fatal("Invalid mesh upload data");

        {
            size_t size = sizeof(Vertex) * meshUploadData.vertices.size();
            vertexBuffer = createBuffer(device, size, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_FLAG_NONE);

            void *mappedData = nullptr;
            const D3D12_RANGE noCpuReadRange{0, 0};

            dxa(vertexBuffer->Map(0, &noCpuReadRange, &mappedData), "Failed to map buffer");

            std::memcpy(mappedData, meshUploadData.vertices.data(), size);
            vertexBuffer->Unmap(0, nullptr);
        }

        {
            size_t size = sizeof(uint32_t) * meshUploadData.indices.size();
            indexBuffer = createBuffer(device, size, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_FLAG_NONE);

            void *mappedData = nullptr;
            const D3D12_RANGE noCpuReadRange{0, 0};

            dxa(indexBuffer->Map(0, &noCpuReadRange, &mappedData), "Failed to map buffer");

            std::memcpy(mappedData, meshUploadData.indices.data(), size);
            indexBuffer->Unmap(0, nullptr);
        }

        D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc{};
        geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
        geometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
        geometryDesc.Triangles.Transform3x4 = 0;
        geometryDesc.Triangles.VertexBuffer.StartAddress = vertexBuffer->GetGPUVirtualAddress();
        geometryDesc.Triangles.VertexBuffer.StrideInBytes = sizeof(Vertex);
        geometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
        geometryDesc.Triangles.VertexCount = vertexCount;
        geometryDesc.Triangles.IndexBuffer = indexBuffer->GetGPUVirtualAddress();
        geometryDesc.Triangles.IndexFormat = DXGI_FORMAT_R32_UINT;
        geometryDesc.Triangles.IndexCount = indexCount;

        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS blasInputs{};
        blasInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
        blasInputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
        blasInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
        blasInputs.NumDescs = 1;
        blasInputs.pGeometryDescs = &geometryDesc;

        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO blasPrebuildInfo{};
        device->GetRaytracingAccelerationStructurePrebuildInfo(&blasInputs, &blasPrebuildInfo);
        if (blasPrebuildInfo.ResultDataMaxSizeInBytes == 0)
            fatal("BLAS prebuild info returned zero size");

        blas.build(device, commandList, blasInputs);
    }
}