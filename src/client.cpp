// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "client.hpp"

#include "command.hpp"

#include "scene/sceneSerialization.hpp"

#include "ui/fileDialog.hpp"

namespace DE
{
    Client::Client(const char *sceneFilePath) : window(L"Dense Engine"),
                                                renderer(window.getHandle(), XMUINT2(window.getWidth(), window.getHeight())),
                                                ui(window.getHandle(), renderer.getDevice(), renderer.getQueue())
    {
        renderer.setViewportRect(makeViewportRect(ui.getSidebarWidthPx()));

        // Default light (sun)
        addDirectionalLight({0.f, 1.f, 0.f}, 1.f, {1.f, 1.f, 1.f});

        if (sceneFilePath != nullptr)
        {
            Handle lastHandle = 0;
            if (loadScene(sceneFilePath, sceneDescription, lastHandle))
            {
                handleGenerator.setLast(lastHandle);
                hasSceneChanged = true;
                renderer.loadScene(sceneDescription);
                this->sceneFilePath = sceneFilePath;
            }
        }
    }

    Client::~Client()
    {
        renderer.waitForGpu();
    }

    void Client::run()
    {
        while (window.tick())
        {
            if (window.getMinimized())
                continue;

            XMUINT2 swapChainSize = renderer.getSwapChainSize();
            XMUINT2 windowSize = XMUINT2(window.getWidth(), window.getHeight());

            bool wasSwapChainResized = false;
            if (swapChainSize.x != windowSize.x || swapChainSize.y != windowSize.y)
            {
                renderer.resizeSwapChain(windowSize);
                renderer.setViewportRect(makeViewportRect(ui.getSidebarWidthPx()));
                wasSwapChainResized = true;
            }

            if (hasSceneChanged)
            {
                renderer.uploadScene(sceneDescription);

                hasSceneChanged = false;
            }

            bool renderThisFrame = false;
            bool requestedScreenshot = false;
            bool isSidebarResized = false;

            std::optional<commands::Command> command;
            ui.tick(sceneDescription, hasSceneChanged, clientState, command, camera, lastFrameFeedback, vsync, script, isScriptRunning, renderThisFrame);
            if (command)
            {
                if (const commands::AddModel *addModelCommand = std::get_if<commands::AddModel>(&*command))
                {
                    addModel(addModelCommand->filePath);
                }
                else if (const commands::AddInstance *addInstanceCommand = std::get_if<commands::AddInstance>(&*command))
                {
                    addModelInstance(addInstanceCommand->modelHandle);
                }
                else if (const commands::AddLight *addLightCommand = std::get_if<commands::AddLight>(&*command))
                {
                    if (addLightCommand->lightType == commands::AddLight::LightType::Point)
                        addPointLight({0.f, 0.f, 0.f}, 1.f, {1.f, 1.f, 1.f}, 5.f);
                    else if (addLightCommand->lightType == commands::AddLight::LightType::Directional)
                        addDirectionalLight({0.f, 1.f, 0.f}, 1.f, {1.f, 1.f, 1.f});
                }
                else if (const commands::LoadScene *loadSceneCommand = std::get_if<commands::LoadScene>(&*command))
                {
                    Handle lastHandle = 0;
                    if (loadScene(loadSceneCommand->filePath, sceneDescription, lastHandle))
                    {
                        handleGenerator.setLast(lastHandle);
                        hasSceneChanged = true;
                        renderer.loadScene(sceneDescription);
                        this->sceneFilePath = loadSceneCommand->filePath;
                    }
                }
                else if (const commands::SaveScene *saveSceneCommand = std::get_if<commands::SaveScene>(&*command))
                {
                    if (this->sceneFilePath == "")
                    {
                        std::optional<std::filesystem::path> selectedPath = saveFileDialog(window.getHandle());
                        if (selectedPath)
                            this->sceneFilePath = selectedPath->string();
                    }

                    saveScene(this->sceneFilePath, sceneDescription);
                }
                else if (const commands::ResizeSidebar *resizeSidebarCommand = std::get_if<commands::ResizeSidebar>(&*command))
                {
                    renderer.setViewportRect(makeViewportRect(resizeSidebarCommand->sidebarWidthPx));
                    isSidebarResized = true;
                }
                else if (const commands::Screenshot *screenshotCommand = std::get_if<commands::Screenshot>(&*command))
                {
                    requestedScreenshot = true;
                }
            }

            if (isScriptRunning)
            {
                hasSceneChanged = true;
                isScriptRunning = scripter::execute(script, sceneDescription);
            }

            clientState.time = std::chrono::duration<float>(std::chrono::steady_clock::now() - startTime).count();

            const bool hasSelectedSceneComponent = clientState.selectedSceneComponentHandle != -1;

            renderThisFrame = !onDemandRendering || renderThisFrame || hasSceneChanged || wasSceneChanged || wasLowerQuality || wasSwapChainResized || requestedScreenshot || isSidebarResized || hasSelectedSceneComponent;

            wasSceneChanged = hasSceneChanged;
            wasLowerQuality = clientState.lowerQuality;

            lastFrameFeedback = renderer.tick(ui, clientState, camera.toConstants(), renderThisFrame, vsync, requestedScreenshot);
        }
    }

    Handle Client::addModel(const std::string &filePath)
    {
        Handle handle = handleGenerator.generateHandle();

        sceneDescription.modelDescriptions.emplace_back(handle, filePath);

        return handle;
    }

    Handle Client::addModelInstance(Handle modelHandle, Transform transform)
    {
        hasSceneChanged = true;

        Handle handle = handleGenerator.generateHandle();

        sceneDescription.modelInstanceDescriptions.emplace_back(handle, modelHandle, transform);

        return handle;
    }

    Handle Client::addPointLight(XMFLOAT3 pos, float intensity, XMFLOAT3 color, float range)
    {
        hasSceneChanged = true;

        Handle handle = handleGenerator.generateHandle();

        sceneDescription.pointLightDescriptions.emplace_back(handle, pos, intensity, color, range);

        return handle;
    }

    Handle Client::addDirectionalLight(XMFLOAT3 direction, float intensity, XMFLOAT3 color)
    {
        hasSceneChanged = true;

        Handle handle = handleGenerator.generateHandle();

        sceneDescription.directionalLightDescriptions.emplace_back(handle, direction, intensity, color);

        return handle;
    }

    Rect Client::makeViewportRect(uint32_t sidebarWidthPx) const
    {
        const XMUINT2 size = renderer.getSwapChainSize();
        sidebarWidthPx = std::min(sidebarWidthPx, size.x - 1);
        return Rect{{sidebarWidthPx, 0}, {size.x - sidebarWidthPx, size.y}};
    }
}