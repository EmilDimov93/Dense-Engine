// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "renderer.hpp"

#include "ui/ui.hpp"

#include "core/dxcommon.hpp"
#include "core/assertions.hpp"

namespace DE
{
    FrameFeedback Renderer::tick(const UI &ui, const ClientState &clientState, const CameraConstants &cameraConstants)
    {
        const UINT frameIndex = swapChain.getCurrentBackBufferIndex();

        commands.begin(frameIndex);

        FrameFeedback frameFeedback;
        void *mapped = nullptr;
        const D3D12_RANGE readRange{0, sizeof(FrameFeedback)};
        dxa(frameFeedbackReadbackBuffers[frameIndex]->Map(0, &readRange, &mapped), "Failed to map frame feedback readback buffer");
        std::memcpy(&frameFeedback, mapped, sizeof(FrameFeedback));
        const D3D12_RANGE noWriteRange{0, 0};
        frameFeedbackReadbackBuffers[frameIndex]->Unmap(0, &noWriteRange);

        ID3D12Resource *backBuffer = swapChain.backBuffers[frameIndex];
        ID3D12Resource *output = renderTarget.texture;

        ID3D12DescriptorHeap *descriptorHeaps[] = {descriptorHeap.get()};
        commands.list->SetDescriptorHeaps(static_cast<UINT>(std::size(descriptorHeaps)), descriptorHeaps);

        SceneSettings sceneSettings = scene.getSceneSettings();

        if (!scene.empty())
        {
            LightCounts lightCounts = scene.getLightCounts();

            commands.list->SetComputeRootSignature(pipeline.globalRootSignature);
            commands.list->SetComputeRootDescriptorTable(dxabstractions::RootParams::OutputTexture, descriptorHeap.gpuHandle(dxabstractions::DESCRIPTOR_HEAP_OUTPUT_UAV_INDEX));
            commands.list->SetComputeRootUnorderedAccessView(dxabstractions::RootParams::FrameFeedback, frameFeedbackBuffer->GetGPUVirtualAddress());
            commands.list->SetComputeRootShaderResourceView(dxabstractions::RootParams::TLAS, scene.getTLASAddress());
            commands.list->SetComputeRootShaderResourceView(dxabstractions::RootParams::MaterialBuffer, scene.getMaterialBufferAddress());
            commands.list->SetComputeRootShaderResourceView(dxabstractions::RootParams::VertexBuffer, scene.getVertexBufferAddress());
            commands.list->SetComputeRootShaderResourceView(dxabstractions::RootParams::IndexBuffer, scene.getIndexBufferAddress());
            commands.list->SetComputeRootShaderResourceView(dxabstractions::RootParams::TLASInstanceDataBuffer, scene.getTLASInstanceDataAddress());
            commands.list->SetComputeRootDescriptorTable(dxabstractions::RootParams::Textures, descriptorHeap.gpuHandle(dxabstractions::DESCRIPTOR_HEAP_FIRST_TEXTURE_INDEX));
            commands.list->SetComputeRootShaderResourceView(dxabstractions::RootParams::PointLightBuffer, scene.getPointLightBufferAddress());
            commands.list->SetComputeRootShaderResourceView(dxabstractions::RootParams::DirectionalLightBuffer, scene.getDirectionalLightBufferAddress());
            commands.list->SetComputeRoot32BitConstants(dxabstractions::RootParams::LightCounts, sizeof(LightCounts) / 4, &lightCounts, 0);
            commands.list->SetComputeRoot32BitConstants(dxabstractions::RootParams::SceneSettings, sizeof(SceneSettings) / 4, &sceneSettings, 0);
            commands.list->SetComputeRoot32BitConstants(dxabstractions::RootParams::Camera, sizeof(CameraConstants) / 4, &cameraConstants, 0);
            commands.list->SetComputeRoot32BitConstants(dxabstractions::RootParams::ClientState, sizeof(ClientState) / 4, &clientState, 0);

            commands.list->SetPipelineState1(pipeline.stateObject);

            D3D12_DISPATCH_RAYS_DESC dispatchDesc = pipeline.dispatchDesc(viewportRect.size);

            const D3D12_WRITEBUFFERIMMEDIATE_PARAMETER resetParams[] = {
                {frameFeedbackBuffer->GetGPUVirtualAddress() + offsetof(FrameFeedback, clickedSceneComponentHandle), static_cast<UINT>(-1)},
                {frameFeedbackBuffer->GetGPUVirtualAddress() + offsetof(FrameFeedback, rayCount), 0},
            };
            commands.list->WriteBufferImmediate(_countof(resetParams), resetParams, nullptr);

            commands.barrier(frameFeedbackBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

            commands.list->DispatchRays(&dispatchDesc);

            commands.barrier(frameFeedbackBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);

            commands.list->CopyResource(frameFeedbackReadbackBuffers[frameIndex], frameFeedbackBuffer);

            commands.barrier(frameFeedbackBuffer, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
        }

        commands.barrier(backBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

        const D3D12_CPU_DESCRIPTOR_HANDLE backBufferView = ui.backBufferView(frameIndex);

        const FLOAT clearColor[4] = {sceneSettings.clearColor.x, sceneSettings.clearColor.y, sceneSettings.clearColor.z, 1.f};
        commands.list->ClearRenderTargetView(backBufferView, clearColor, 0, nullptr);

        commands.barrier(output, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
        commands.barrier(backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_DEST);

        D3D12_TEXTURE_COPY_LOCATION dst{};
        dst.pResource = backBuffer;
        dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        dst.SubresourceIndex = 0;

        D3D12_TEXTURE_COPY_LOCATION src{};
        src.pResource = output;
        src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        src.SubresourceIndex = 0;

        const XMUINT2 swapChainSize = swapChain.getSize();
        if (!scene.empty() && viewportRect.start.x < swapChainSize.x && viewportRect.start.y < swapChainSize.y)
        {
            D3D12_BOX srcBox{};
            srcBox.right = std::min(viewportRect.size.x, swapChainSize.x - viewportRect.start.x);
            srcBox.bottom = std::min(viewportRect.size.y, swapChainSize.y - viewportRect.start.y);
            srcBox.back = 1;

            commands.list->CopyTextureRegion(&dst, viewportRect.start.x, viewportRect.start.y, 0, &src, &srcBox);
        }

        commands.barrier(output, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        commands.barrier(backBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET);

        ui.draw(commands.list, frameIndex);

        commands.barrier(backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

        commands.execute(frameIndex);

        swapChain.present();

        return frameFeedback;
    }

    void Renderer::loadScene(const SceneDescription &sceneDescription)
    {
        commands.waitIdle();
        scene.clear();
        uploadScene(sceneDescription);
    }

    void Renderer::uploadScene(const SceneDescription &sceneDescription)
    {
        commands.waitIdle();

        commands.begin(0);

        scene.upload(sceneDescription);

        commands.execute(0);
        commands.waitIdle();

        scene.releaseUploadBuffers();
    }

    XMUINT2 Renderer::getSwapChainSize() const
    {
        return swapChain.getSize();
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
        renderTarget = dxabstractions::RenderTarget(device, descriptorHeap.cpuHandle(dxabstractions::DESCRIPTOR_HEAP_OUTPUT_UAV_INDEX), rect.size.x, rect.size.y);
    }
}