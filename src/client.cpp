// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "client.hpp"

#include "command.hpp"

#include "scene/sceneSerialization.hpp"

namespace DE
{
    Client::Client(const char *sceneFilePath) : window(L"Dense Engine"),
                                                renderer(window.getHandle(), XMUINT2(window.getWidth(), window.getHeight())),
                                                ui(window.getHandle(), renderer.getDevice(), renderer.getQueue(), renderer.getBackBuffers())
    {
        renderer.setViewportRect(makeViewportRect());

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

            if (swapChainSize.x != windowSize.x || swapChainSize.y != windowSize.y)
            {
                renderer.resizeSwapChain(windowSize);
                ui.createBackBufferViews(renderer.getBackBuffers());
                renderer.setViewportRect(makeViewportRect());
            }

            if (hasSceneChanged)
            {
                renderer.uploadScene(sceneDescription);

                hasSceneChanged = false;
            }

            std::optional<commands::Command> command;
            ui.tick(makeViewportRect().start.x, sceneDescription, hasSceneChanged, clientState, command, camera);
            if (command)
            {
                if (const commands::AddModelCommand *addModelCommand = std::get_if<commands::AddModelCommand>(&*command))
                {
                    addModel(addModelCommand->filePath);
                }
                else if (const commands::AddInstanceCommand *addInstanceCommand = std::get_if<commands::AddInstanceCommand>(&*command))
                {
                    addModelInstance(addInstanceCommand->modelHandle);
                }
                else if (const commands::AddLightCommand *addLightCommand = std::get_if<commands::AddLightCommand>(&*command))
                {
                    if(addLightCommand->lightType == commands::AddLightCommand::LightType::Point)
                        addPointLight({0.f, 0.f, 0.f}, 1.f, {1.f, 1.f, 1.f}, 5.f);
                    else if(addLightCommand->lightType == commands::AddLightCommand::LightType::Directional)
                        addDirectionalLight({0.f, 1.f, 0.f}, 1.f, {1.f, 1.f, 1.f});
                }
                else if (const commands::LoadSceneCommand *loadSceneCommand = std::get_if<commands::LoadSceneCommand>(&*command))
                {
                    Handle lastHandle = 0;
                    if (loadScene(loadSceneCommand->filePath, sceneDescription, lastHandle))
                    {
                        handleGenerator.setLast(lastHandle);
                        hasSceneChanged = true;
                        renderer.loadScene(sceneDescription);
                    }
                }
                else if (const commands::SaveSceneCommand *saveSceneCommand = std::get_if<commands::SaveSceneCommand>(&*command))
                {
                    saveScene("scene.descene", sceneDescription);
                }
            }

            clientState.time = std::chrono::duration<float>(std::chrono::steady_clock::now() - startTime).count();

            renderer.tick(ui, clientState, camera.toConstants());
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

    void Client::setSidebarWidthFraction(float fraction)
    {
        if (fraction >= 0.f && fraction < 1.f)
        {
            sidebarWidthFraction = fraction;
            renderer.setViewportRect(makeViewportRect());
        }
    }

    Rect Client::makeViewportRect() const
    {
        XMUINT2 swapChainSize = renderer.getSwapChainSize();

        return Rect{{static_cast<uint32_t>(sidebarWidthFraction * swapChainSize.x), 0},
                    {static_cast<uint32_t>(swapChainSize.x - sidebarWidthFraction * swapChainSize.x), swapChainSize.y}};
    }
}