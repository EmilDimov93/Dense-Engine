// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "renderer.hpp"

#include "../ui/ui.hpp"

#include "assertions.hpp"

namespace DE
{
    void Renderer::tick(const UI &ui, const Settings &settings)
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
        commands.list->SetComputeRoot32BitConstants(abstractions::RootParams::Settings, 2, &settings, 0);
        commands.list->SetPipelineState1(pipeline.stateObject.Get());

        const UINT64 shaderRecordSize = D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT;

        D3D12_DISPATCH_RAYS_DESC dispatchDesc{};
        dispatchDesc.RayGenerationShaderRecord.StartAddress = pipeline.Raygen();
        dispatchDesc.RayGenerationShaderRecord.SizeInBytes = shaderRecordSize;
        dispatchDesc.MissShaderTable.StartAddress = pipeline.Miss();
        dispatchDesc.MissShaderTable.SizeInBytes = 2 * shaderRecordSize;
        dispatchDesc.MissShaderTable.StrideInBytes = shaderRecordSize;
        dispatchDesc.HitGroupTable.StartAddress = pipeline.HitGroup();
        dispatchDesc.HitGroupTable.SizeInBytes = shaderRecordSize;
        dispatchDesc.HitGroupTable.StrideInBytes = shaderRecordSize;
        dispatchDesc.Width = viewportRect.size.x;
        dispatchDesc.Height = viewportRect.size.y;
        dispatchDesc.Depth = 1;

        commands.list->DispatchRays(&dispatchDesc);

        const D3D12_CPU_DESCRIPTOR_HANDLE backBufferView = ui.backBufferView(frameIndex);

        const D3D12_RESOURCE_BARRIER toClearBarrier = transition(backBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
        commands.list->ResourceBarrier(1, &toClearBarrier);

        const float clearColor[] = {0.1f, 0.1f, 0.1f, 1.0f};
        commands.list->ClearRenderTargetView(backBufferView, clearColor, 0, nullptr);

        const D3D12_RESOURCE_BARRIER toCopyBarriers[] = {
            transition(output, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE),
            transition(backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_DEST)};
        commands.list->ResourceBarrier(static_cast<UINT>(std::size(toCopyBarriers)), toCopyBarriers);

        D3D12_TEXTURE_COPY_LOCATION dst{};
        dst.pResource = backBuffer;
        dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        dst.SubresourceIndex = 0;

        D3D12_TEXTURE_COPY_LOCATION src{};
        src.pResource = output;
        src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        src.SubresourceIndex = 0;

        if (viewportRect.start.x < swapChain.size.x && viewportRect.start.y < swapChain.size.y)
        {
            D3D12_BOX srcBox{};
            srcBox.right = std::min(viewportRect.size.x, swapChain.size.x - viewportRect.start.x);
            srcBox.bottom = std::min(viewportRect.size.y, swapChain.size.y - viewportRect.start.y);
            srcBox.back = 1;

            commands.list->CopyTextureRegion(&dst, viewportRect.start.x, viewportRect.start.y, 0, &src, &srcBox);
        }

        const D3D12_RESOURCE_BARRIER toGuiBarriers[] = {
            transition(output, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS),
            transition(backBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET)};
        commands.list->ResourceBarrier(static_cast<UINT>(std::size(toGuiBarriers)), toGuiBarriers);

        ui.draw(commands.list.Get(), frameIndex);

        const D3D12_RESOURCE_BARRIER toPresentBarrier = transition(backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
        commands.list->ResourceBarrier(1, &toPresentBarrier);

        commands.execute(frameIndex);

        dxa(swapChain.swapchain->Present(1, 0), "Failed to present");
    }

    void Renderer::uploadScene(const SceneDescription &sceneDescription)
    {
        for (abstractions::Fence &fence : commands.fences)
            fence.wait();

        commands.begin(0);

        scene.upload(sceneDescription);

        commands.execute(0);
        commands.fences[0].wait();
    }

    XMUINT2 Renderer::getSwapChainSize() const
    {
        return swapChain.size;
    }

    void Renderer::resizeSwapChain(XMUINT2 size)
    {
        for (abstractions::Fence &fence : commands.fences)
            fence.wait();

        swapChain.resize(size);
    }

    void Renderer::setViewportRect(Rect rect)
    {
        viewportRect.start = rect.start;

        for (abstractions::Fence &fence : commands.fences)
            fence.wait();

        viewportRect.size = rect.size;
        renderTarget = abstractions::RenderTarget(device, rect.size.x, rect.size.y);
    }
}