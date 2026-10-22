// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "core/dxcommon.hpp"
#include "core/assertions.hpp"

#include <cstring>
#include <fstream>
#include <vector>

namespace DE::dxabstractions
{
    enum RootParams
    {
        OutputTexture = 0,
        TLAS = 1,
        MaterialBuffer = 2,
        VertexBuffer = 3,
        IndexBuffer = 4,
        TLASInstanceDataBuffer = 5,
        PointLightBuffer = 6,
        DirectionalLightBuffer = 7,
        LightCounts = 8,
        ClientState = 9,
        Count
    };

    struct Pipeline
    {
        ComPtr<ID3D12RootSignature> globalRootSignature;
        ComPtr<ID3D12StateObject> stateObject;
        ComPtr<ID3D12Resource> shaderTable;

        Pipeline(ID3D12Device5 *device, const char *shaderFilePath)
        {
            std::ifstream file(shaderFilePath, std::ios::binary | std::ios::ate);
            if (!file)
                fatal("Failed to open shader file");
            
            std::vector<char> bytecode(static_cast<size_t>(file.tellg()));
            file.seekg(0);
            file.read(bytecode.data(), bytecode.size());

            D3D12_DESCRIPTOR_RANGE uavRange{
                .RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV,
                .NumDescriptors = 1,
                .BaseShaderRegister = 0,
                .RegisterSpace = 0,
                .OffsetInDescriptorsFromTableStart = 0};

            D3D12_ROOT_PARAMETER rootParams[RootParams::Count] = {};
            rootParams[RootParams::OutputTexture].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            rootParams[RootParams::OutputTexture].DescriptorTable = {1, &uavRange};
            rootParams[RootParams::OutputTexture].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
            rootParams[RootParams::TLAS].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
            rootParams[RootParams::TLAS].Descriptor = {0, 0};
            rootParams[RootParams::TLAS].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
            rootParams[RootParams::MaterialBuffer].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
            rootParams[RootParams::MaterialBuffer].Descriptor = {1, 0};
            rootParams[RootParams::MaterialBuffer].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
            rootParams[RootParams::VertexBuffer].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
            rootParams[RootParams::VertexBuffer].Descriptor = {2, 0};
            rootParams[RootParams::VertexBuffer].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
            rootParams[RootParams::IndexBuffer].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
            rootParams[RootParams::IndexBuffer].Descriptor = {3, 0};
            rootParams[RootParams::IndexBuffer].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
            rootParams[RootParams::TLASInstanceDataBuffer].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
            rootParams[RootParams::TLASInstanceDataBuffer].Descriptor = {4, 0};
            rootParams[RootParams::TLASInstanceDataBuffer].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
            rootParams[RootParams::PointLightBuffer].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
            rootParams[RootParams::PointLightBuffer].Descriptor = {5, 0};
            rootParams[RootParams::PointLightBuffer].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
            rootParams[RootParams::DirectionalLightBuffer].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
            rootParams[RootParams::DirectionalLightBuffer].Descriptor = {6, 0};
            rootParams[RootParams::DirectionalLightBuffer].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
            rootParams[RootParams::LightCounts].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
            rootParams[RootParams::LightCounts].Constants = {
                .ShaderRegister = 0,
                .RegisterSpace = 0,
                .Num32BitValues = 2};
            rootParams[RootParams::ClientState].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
            rootParams[RootParams::ClientState].Constants = {
                .ShaderRegister = 1,
                .RegisterSpace = 0,
                .Num32BitValues = 3};

            D3D12_ROOT_SIGNATURE_DESC rootSigDesc{.NumParameters = RootParams::Count, .pParameters = rootParams, .NumStaticSamplers = 0, .pStaticSamplers = nullptr, .Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE};

            ComPtr<ID3DBlob> serialized, error;
            dxa(D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &serialized, &error), "Failed to serialize root signature");
            dxa(device->CreateRootSignature(0, serialized->GetBufferPointer(), serialized->GetBufferSize(), IID_PPV_ARGS(&globalRootSignature)), "Failed to create root signature");

            D3D12_DXIL_LIBRARY_DESC dxilLib{
                .DXILLibrary = {bytecode.data(), bytecode.size()},
                .NumExports = 0,
                .pExports = nullptr};

            D3D12_HIT_GROUP_DESC hitGroup{
                .HitGroupExport = L"HitGroup",
                .Type = D3D12_HIT_GROUP_TYPE_TRIANGLES,
                .ClosestHitShaderImport = L"ClosestHit"};

            D3D12_RAYTRACING_SHADER_CONFIG shaderConfig{
                .MaxPayloadSizeInBytes = 3 * sizeof(float) + sizeof(uint32_t),
                .MaxAttributeSizeInBytes = 2 * sizeof(float)};

            D3D12_GLOBAL_ROOT_SIGNATURE globalSig{globalRootSignature.Get()};

            D3D12_RAYTRACING_PIPELINE_CONFIG pipelineConfig{.MaxTraceRecursionDepth = 3};

            const D3D12_STATE_SUBOBJECT subobjects[] =
                {
                    {D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY, &dxilLib},
                    {D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP, &hitGroup},
                    {D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG, &shaderConfig},
                    {D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE, &globalSig},
                    {D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG, &pipelineConfig}};

            D3D12_STATE_OBJECT_DESC stateObjectDesc{
                .Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE,
                .NumSubobjects = static_cast<UINT>(std::size(subobjects)),
                .pSubobjects = subobjects};

            dxa(device->CreateStateObject(&stateObjectDesc, IID_PPV_ARGS(&stateObject)), "Failed to create raytracing state object");

            ComPtr<ID3D12StateObjectProperties> props;
            dxa(stateObject.As(&props), "Failed to get state object properties");

            {
                D3D12_HEAP_PROPERTIES heapProps{.Type = D3D12_HEAP_TYPE_UPLOAD};

                D3D12_RESOURCE_DESC bufferDesc{
                    .Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
                    .Alignment = 0,
                    .Width = 4 * D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT,
                    .Height = 1,
                    .DepthOrArraySize = 1,
                    .MipLevels = 1,
                    .Format = DXGI_FORMAT_UNKNOWN,
                    .SampleDesc = {1, 0},
                    .Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
                    .Flags = D3D12_RESOURCE_FLAG_NONE};

                ComPtr<ID3D12Resource> buffer;
                dxa(device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&buffer)), "Failed to create buffer");
                shaderTable = buffer;
            }

            uint8_t *mapped = nullptr;
            dxa(shaderTable->Map(0, nullptr, reinterpret_cast<void **>(&mapped)), "Failed to map shader table");
            std::memcpy(mapped + 0 * D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT, props->GetShaderIdentifier(L"RayGen"), D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
            std::memcpy(mapped + 1 * D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT, props->GetShaderIdentifier(L"Miss"), D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
            std::memcpy(mapped + 2 * D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT, props->GetShaderIdentifier(L"ShadowMiss"), D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
            std::memcpy(mapped + 3 * D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT, props->GetShaderIdentifier(L"HitGroup"), D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
            shaderTable->Unmap(0, nullptr);
        }

        D3D12_DISPATCH_RAYS_DESC dispatchDesc(XMUINT2 size) const
        {
            D3D12_DISPATCH_RAYS_DESC dispatchDesc{};
            dispatchDesc.RayGenerationShaderRecord.StartAddress = Raygen();
            dispatchDesc.RayGenerationShaderRecord.SizeInBytes = D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT;
            dispatchDesc.MissShaderTable.StartAddress = Miss();
            dispatchDesc.MissShaderTable.SizeInBytes = 2 * D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT;
            dispatchDesc.MissShaderTable.StrideInBytes = D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT;
            dispatchDesc.HitGroupTable.StartAddress = HitGroup();
            dispatchDesc.HitGroupTable.SizeInBytes = D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT;
            dispatchDesc.HitGroupTable.StrideInBytes = D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT;
            dispatchDesc.Width = size.x;
            dispatchDesc.Height = size.y;
            dispatchDesc.Depth = 1;

            return dispatchDesc;
        }

        D3D12_GPU_VIRTUAL_ADDRESS Raygen() const { return shaderTable->GetGPUVirtualAddress(); }
        D3D12_GPU_VIRTUAL_ADDRESS Miss() const { return shaderTable->GetGPUVirtualAddress() + D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT; }
        D3D12_GPU_VIRTUAL_ADDRESS ShadowMiss() const { return shaderTable->GetGPUVirtualAddress() + 2 * D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT; }
        D3D12_GPU_VIRTUAL_ADDRESS HitGroup() const { return shaderTable->GetGPUVirtualAddress() + 3 * D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT; }
    };
}