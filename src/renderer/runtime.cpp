// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "renderer.hpp"

#include "ui/ui.hpp"

#include "core/dxcommon.hpp"
#include "core/assertions.hpp"

namespace DE
{
    void Renderer::tick(const UI &ui, const Settings &settings)
    {
        const UINT frameIndex = swapChain.swapchain->GetCurrentBackBufferIndex();

        commands.begin(frameIndex);

        ID3D12Resource *backBuffer = swapChain.backBuffers[frameIndex].Get();
        ID3D12Resource *output = renderTarget.texture.Get();

        ID3D12DescriptorHeap *descriptorHeaps[] = {renderTarget.descriptorHeap.Get()};
        commands.list->SetDescriptorHeaps(static_cast<UINT>(std::size(descriptorHeaps)), descriptorHeaps);

        if (!scene.empty())
        {
            commands.list->SetComputeRootSignature(pipeline.globalRootSignature.Get());
            commands.list->SetComputeRootDescriptorTable(dxabstractions::RootParams::OutputTexture, renderTarget.descriptorHeap->GetGPUDescriptorHandleForHeapStart());
            commands.list->SetComputeRootShaderResourceView(dxabstractions::RootParams::TLAS, scene.getTLASAddress());
            commands.list->SetComputeRootShaderResourceView(dxabstractions::RootParams::MaterialBuffer, scene.getMaterialBufferAddress());
            commands.list->SetComputeRootShaderResourceView(dxabstractions::RootParams::VertexBuffer, scene.getVertexBufferAddress());
            commands.list->SetComputeRootShaderResourceView(dxabstractions::RootParams::IndexBuffer, scene.getIndexBufferAddress());
            commands.list->SetComputeRootShaderResourceView(dxabstractions::RootParams::TLASInstanceDataBuffer, scene.getTLASInstanceDataAddress());
            commands.list->SetComputeRoot32BitConstants(dxabstractions::RootParams::Settings, 2, &settings, 0);
            commands.list->SetPipelineState1(pipeline.stateObject.Get());

            D3D12_DISPATCH_RAYS_DESC dispatchDesc = pipeline.dispatchDesc(viewportRect.size);

            commands.list->DispatchRays(&dispatchDesc);
        }

        const D3D12_RESOURCE_BARRIER toClearBarrier = transition(backBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
        commands.list->ResourceBarrier(1, &toClearBarrier);

        const D3D12_CPU_DESCRIPTOR_HANDLE backBufferView = ui.backBufferView(frameIndex);

        const float clearColor[] = {0.5f, 0.5f, 0.8f, 1.0f};
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

        if (!scene.empty() && viewportRect.start.x < swapChain.size.x && viewportRect.start.y < swapChain.size.y)
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
        commands.waitIdle();

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
        commands.waitIdle();

        swapChain.resize(size);
    }

    void Renderer::setViewportRect(Rect rect)
    {
        viewportRect.start = rect.start;

        commands.waitIdle();

        viewportRect.size = rect.size;
        renderTarget = dxabstractions::RenderTarget(device, rect.size.x, rect.size.y);
    }
}