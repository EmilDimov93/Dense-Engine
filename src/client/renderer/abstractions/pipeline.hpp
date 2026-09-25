// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "../common.hpp"

#include <cstring>
#include <fstream>
#include <vector>

namespace DE::abstractions
{
    struct Pipeline
    {
        ComPtr<ID3D12RootSignature> globalRootSignature;
        ComPtr<ID3D12StateObject> stateObject;
        ComPtr<ID3D12Resource> shaderTable;

        Pipeline(ID3D12Device5 *device, const char *shaderFilePath)
        {
            std::ifstream file(shaderFilePath, std::ios::binary | std::ios::ate);
            if (!file)
            {
                std::cerr << "Failed to open shader file" << std::endl;
                abort();
            }
            std::vector<char> bytecode(static_cast<size_t>(file.tellg()));
            file.seekg(0);
            file.read(bytecode.data(), bytecode.size());

            D3D12_DESCRIPTOR_RANGE uavRange{
                .RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV,
                .NumDescriptors = 1,
                .BaseShaderRegister = 0,
                .RegisterSpace = 0,
                .OffsetInDescriptorsFromTableStart = 0};

            D3D12_ROOT_PARAMETER rootParams[2] = {};
            rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
            rootParams[0].Descriptor = {0, 0};
            rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
            rootParams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            rootParams[1].DescriptorTable = {1, &uavRange};
            rootParams[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

            D3D12_ROOT_SIGNATURE_DESC rootSigDesc{
                .NumParameters = 2,
                .pParameters = rootParams,
                .NumStaticSamplers = 0,
                .pStaticSamplers = nullptr,
                .Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE};

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
                .MaxPayloadSizeInBytes = 3 * sizeof(float),
                .MaxAttributeSizeInBytes = 2 * sizeof(float)
            };

            D3D12_GLOBAL_ROOT_SIGNATURE globalSig{globalRootSignature.Get()};

            D3D12_RAYTRACING_PIPELINE_CONFIG pipelineConfig{.MaxTraceRecursionDepth = 1};

            const D3D12_STATE_SUBOBJECT subobjects[] =
                {
                    {D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY, &dxilLib},
                    {D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP, &hitGroup},
                    {D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG, &shaderConfig},
                    {D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE, &globalSig},
                    {D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG, &pipelineConfig}};

            D3D12_STATE_OBJECT_DESC stateObjectDesc{
                .Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE,
                .NumSubobjects = _countof(subobjects),
                .pSubobjects = subobjects};

            dxa(device->CreateStateObject(&stateObjectDesc, IID_PPV_ARGS(&stateObject)), "Failed to create raytracing state object");

            ComPtr<ID3D12StateObjectProperties> props;
            dxa(stateObject.As(&props), "Failed to get state object properties");

            {
                D3D12_HEAP_PROPERTIES heapProps{.Type = D3D12_HEAP_TYPE_UPLOAD};

                D3D12_RESOURCE_DESC bufferDesc{
                    .Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
                    .Alignment = 0,
                    .Width = 3 * D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT,
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
            std::memcpy(mapped + 2 * D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT, props->GetShaderIdentifier(L"HitGroup"), D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
            shaderTable->Unmap(0, nullptr);
        }

        D3D12_GPU_VIRTUAL_ADDRESS Raygen() const { return shaderTable->GetGPUVirtualAddress(); }
        D3D12_GPU_VIRTUAL_ADDRESS Miss() const { return shaderTable->GetGPUVirtualAddress() + D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT; }
        D3D12_GPU_VIRTUAL_ADDRESS HitGroup() const { return shaderTable->GetGPUVirtualAddress() + 2 * D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT; }
    };
}