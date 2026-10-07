// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "renderer.hpp"

#include "assertions.hpp"

namespace DE
{
    void Renderer::tick()
    {
        const UINT frameIndex = swapChain.swapchain->GetCurrentBackBufferIndex();

        commands.begin(frameIndex);

        ID3D12Resource *backBuffer = swapChain.backBuffers[frameIndex].Get();
        ID3D12Resource *output = renderTarget.texture.Get();

        auto transition = [](ID3D12Resource *resource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after) -> D3D12_RESOURCE_BARRIER
        {
            D3D12_RESOURCE_BARRIER barrier{};
            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barrier.Transition.pResource = resource;
            barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            barrier.Transition.StateBefore = before;
            barrier.Transition.StateAfter = after;
            return barrier;
        };

        ID3D12DescriptorHeap *descriptorHeaps[] = {renderTarget.descriptorHeap.Get()};
        commands.list->SetDescriptorHeaps(static_cast<UINT>(std::size(descriptorHeaps)), descriptorHeaps);

        commands.list->SetComputeRootSignature(pipeline.globalRootSignature.Get());
        commands.list->SetComputeRootDescriptorTable(abstractions::RootParams::OutputTexture, renderTarget.descriptorHeap->GetGPUDescriptorHandleForHeapStart());
        commands.list->SetComputeRootShaderResourceView(abstractions::RootParams::TLAS, scene.getTLASAddress());
        commands.list->SetComputeRootShaderResourceView(abstractions::RootParams::MaterialBuffer, scene.getMaterialsAddress());
        commands.list->SetPipelineState1(pipeline.stateObject.Get());

        const UINT64 shaderRecordSize = D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT;

        D3D12_DISPATCH_RAYS_DESC dispatchDesc{};
        dispatchDesc.RayGenerationShaderRecord.StartAddress = pipeline.Raygen();
        dispatchDesc.RayGenerationShaderRecord.SizeInBytes = shaderRecordSize;
        dispatchDesc.MissShaderTable.StartAddress = pipeline.Miss();
        dispatchDesc.MissShaderTable.SizeInBytes = shaderRecordSize;
        dispatchDesc.MissShaderTable.StrideInBytes = shaderRecordSize;
        dispatchDesc.HitGroupTable.StartAddress = pipeline.HitGroup();
        dispatchDesc.HitGroupTable.SizeInBytes = shaderRecordSize;
        dispatchDesc.HitGroupTable.StrideInBytes = shaderRecordSize;
        dispatchDesc.Width = swapChain.width;
        dispatchDesc.Height = swapChain.height;
        dispatchDesc.Depth = 1;

        commands.list->DispatchRays(&dispatchDesc);

        const D3D12_RESOURCE_BARRIER toCopyBarriers[] = {
            transition(output, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE),
            transition(backBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_COPY_DEST)};
        commands.list->ResourceBarrier(static_cast<UINT>(std::size(toCopyBarriers)), toCopyBarriers);

        commands.list->CopyResource(backBuffer, output);

        const D3D12_RESOURCE_BARRIER toPresentBarriers[] = {
            transition(output, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS),
            transition(backBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PRESENT)};
        commands.list->ResourceBarrier(static_cast<UINT>(std::size(toPresentBarriers)), toPresentBarriers);

        commands.execute(frameIndex);

        dxa(swapChain.swapchain->Present(1, 0), "Failed to present");
    }
}