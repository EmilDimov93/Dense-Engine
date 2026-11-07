// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "renderer.hpp"

#include "ui/ui.hpp"

#include "core/dxcommon.hpp"
#include "core/assertions.hpp"

#include <fstream>

namespace DE
{
    gpu::FrameFeedback Renderer::tick(const UI &ui, gpu::ClientState clientState, const gpu::CameraConstants &cameraConstants, bool renderThisFrame, bool vsync, bool screenshot)
    {
        const UINT frameIndex = swapChain.getCurrentBackBufferIndex();

        commands.begin(frameIndex);

        gpu::FrameFeedback frameFeedback;
        void *mapped = nullptr;
        const D3D12_RANGE readRange{0, sizeof(gpu::FrameFeedback)};
        dxa(frameFeedbackReadbackBuffers[frameIndex]->Map(0, &readRange, &mapped), "Failed to map frame feedback readback buffer");
        std::memcpy(&frameFeedback, mapped, sizeof(gpu::FrameFeedback));
        const gpu::FrameFeedback cleared{};
        std::memcpy(mapped, &cleared, sizeof(cleared));
        const D3D12_RANGE writeRange{0, sizeof(gpu::FrameFeedback)};
        frameFeedbackReadbackBuffers[frameIndex]->Unmap(0, &writeRange);

        ID3D12Resource *backBuffer = swapChain.backBuffer(frameIndex);
        ID3D12Resource *output = renderTarget.texture;

        ID3D12DescriptorHeap *descriptorHeaps[] = {descriptorHeap.get()};
        commands.list->SetDescriptorHeaps(static_cast<UINT>(std::size(descriptorHeaps)), descriptorHeaps);

        gpu::Environment environment = scene.getEnvironment();

        if (!scene.empty() && renderThisFrame)
        {
            gpu::LightCounts lightCounts = scene.getLightCounts();

            if (screenshot)
            {
                clientState.lowerQuality = false;
                clientState.ssaaSamples = 16u;
            }

            commands.list->SetComputeRootSignature(pipeline.globalRootSignature);
            commands.list->SetComputeRootDescriptorTable(dxwrap::RootParams::OutputTexture, descriptorHeap.gpuHandle(dxwrap::DESCRIPTOR_HEAP_OUTPUT_UAV_INDEX));
            commands.list->SetComputeRootUnorderedAccessView(dxwrap::RootParams::FrameFeedback, frameFeedbackBuffer->GetGPUVirtualAddress());
            commands.list->SetComputeRootShaderResourceView(dxwrap::RootParams::TLAS, scene.getTLASAddress());
            commands.list->SetComputeRootShaderResourceView(dxwrap::RootParams::MaterialBuffer, scene.getMaterialBufferAddress());
            commands.list->SetComputeRootShaderResourceView(dxwrap::RootParams::VertexBuffer, scene.getVertexBufferAddress());
            commands.list->SetComputeRootShaderResourceView(dxwrap::RootParams::IndexBuffer, scene.getIndexBufferAddress());
            commands.list->SetComputeRootShaderResourceView(dxwrap::RootParams::TLASInstanceDataBuffer, scene.getTLASInstanceDataAddress());
            commands.list->SetComputeRootDescriptorTable(dxwrap::RootParams::Textures, descriptorHeap.gpuHandle(dxwrap::DESCRIPTOR_HEAP_FIRST_TEXTURE_INDEX));
            commands.list->SetComputeRootShaderResourceView(dxwrap::RootParams::PointLightBuffer, scene.getPointLightBufferAddress());
            commands.list->SetComputeRootShaderResourceView(dxwrap::RootParams::DirectionalLightBuffer, scene.getDirectionalLightBufferAddress());
            commands.list->SetComputeRoot32BitConstants(dxwrap::RootParams::LightCounts, sizeof(gpu::LightCounts) / 4, &lightCounts, 0);
            commands.list->SetComputeRoot32BitConstants(dxwrap::RootParams::Environment, sizeof(gpu::Environment) / 4, &environment, 0);
            commands.list->SetComputeRoot32BitConstants(dxwrap::RootParams::Camera, sizeof(gpu::CameraConstants) / 4, &cameraConstants, 0);
            commands.list->SetComputeRoot32BitConstants(dxwrap::RootParams::ClientState, sizeof(gpu::ClientState) / 4, &clientState, 0);

            commands.list->SetPipelineState1(pipeline.stateObject);

            D3D12_DISPATCH_RAYS_DESC dispatchDesc = pipeline.dispatchDesc(viewportRect.size);

            const D3D12_WRITEBUFFERIMMEDIATE_PARAMETER resetParams[] = {
                {frameFeedbackBuffer->GetGPUVirtualAddress() + offsetof(gpu::FrameFeedback, clickedSceneComponentHandle), static_cast<UINT>(-1)},
                {frameFeedbackBuffer->GetGPUVirtualAddress() + offsetof(gpu::FrameFeedback, rayCount), 0},
            };
            commands.list->WriteBufferImmediate(_countof(resetParams), resetParams, nullptr);

            commands.barrier(frameFeedbackBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

            commands.list->DispatchRays(&dispatchDesc);

            commands.barrier(frameFeedbackBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);

            commands.list->CopyResource(frameFeedbackReadbackBuffers[frameIndex], frameFeedbackBuffer);

            commands.barrier(frameFeedbackBuffer, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
        }

        commands.barrier(backBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

        const D3D12_CPU_DESCRIPTOR_HANDLE backBufferView = swapChain.backBufferView(frameIndex);

        const FLOAT clearColor[4] = {environment.clearColor.x, environment.clearColor.y, environment.clearColor.z, 1.f};
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

        ui.draw(commands.list, backBufferView);

        ID3D12ResourcePtr captureBuffer;
        D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint{};

        if (screenshot)
        {
            const D3D12_RESOURCE_DESC backBufferDesc = backBuffer->GetDesc();
            UINT64 totalBytes = 0;
            device.device->GetCopyableFootprints(&backBufferDesc, 0, 1, 0, &footprint, nullptr, nullptr, &totalBytes);
            captureBuffer = createBuffer(device, totalBytes, D3D12_HEAP_TYPE_READBACK, D3D12_RESOURCE_STATE_COPY_DEST);

            commands.barrier(backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE);

            D3D12_TEXTURE_COPY_LOCATION capDst{};
            capDst.pResource = captureBuffer;
            capDst.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
            capDst.PlacedFootprint = footprint;

            D3D12_TEXTURE_COPY_LOCATION capSrc{};
            capSrc.pResource = backBuffer;
            capSrc.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
            capSrc.SubresourceIndex = 0;

            commands.list->CopyTextureRegion(&capDst, 0, 0, 0, &capSrc, nullptr);

            commands.barrier(backBuffer, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_PRESENT);
        }
        else
        {
            commands.barrier(backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
        }

        commands.execute(frameIndex);
        swapChain.present(vsync);

        if (screenshot)
        {
            waitForGpu();

            void *data = nullptr;
            dxa(captureBuffer->Map(0, nullptr, &data), "Failed to map capture buffer");

            std::ofstream file("output.ppm", std::ios::binary);
            file << "P6\n"
                 << swapChainSize.x << " " << swapChainSize.y << "\n255\n";

            const uint8_t *base = static_cast<const uint8_t *>(data) + footprint.Offset;
            for (UINT y = 0; y < swapChainSize.y; ++y)
            {
                const uint8_t *row = base + y * footprint.Footprint.RowPitch;
                for (UINT x = 0; x < swapChainSize.x; ++x)
                    file.write(reinterpret_cast<const char *>(row + x * 4), 3);
            }

            const D3D12_RANGE noWrite{0, 0};
            captureBuffer->Unmap(0, &noWrite);
        }

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

        swapChain.resize(size, device);
    }

    void Renderer::setViewportRect(Rect rect)
    {
        viewportRect.start = rect.start;

        commands.waitIdle();

        viewportRect.size = rect.size;
        renderTarget = dxwrap::RenderTarget(device, descriptorHeap.cpuHandle(dxwrap::DESCRIPTOR_HEAP_OUTPUT_UAV_INDEX), rect.size.x, rect.size.y);
    }
}