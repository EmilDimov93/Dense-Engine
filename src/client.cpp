// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "client.hpp"

#include "command.hpp"

#include "scene/sceneSerialization.hpp"

namespace DE
{
    Rect Client::makeViewportRect() const
    {
        XMUINT2 swapChainSize = renderer.getSwapChainSize();

        return Rect{{static_cast<uint32_t>(sidebarWidthFraction * swapChainSize.x), 0},
                    {static_cast<uint32_t>(swapChainSize.x - sidebarWidthFraction * swapChainSize.x), swapChainSize.y}};
    }

    Client::Client(const char *sceneFilePath) : window(L"Dense Engine"),
                       renderer(window.getHandle(), XMUINT2(window.getWidth(), window.getHeight())),
                       ui(window.getHandle(), renderer.getDevice(), renderer.getQueue(), renderer.getBackBuffers())
    {
        renderer.setViewportRect(makeViewportRect());

        if(sceneFilePath != "")
            loadScene(sceneFilePath, sceneDescription);
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
            ui.tick(makeViewportRect().start.x, sceneDescription, hasSceneChanged, settings, command);
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
                else if(const commands::LoadSceneCommand *loadSceneCommand = std::get_if<commands::LoadSceneCommand>(&*command))
                {
                    loadScene(loadSceneCommand->filePath, sceneDescription);
                    hasSceneChanged = true;
                }
                else if(const commands::SaveSceneCommand *saveSceneCommand = std::get_if<commands::SaveSceneCommand>(&*command))
                {
                    saveScene("scene.descene", sceneDescription);
                }
            }

            renderer.tick(ui, settings);
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

    void Client::setSidebarWidthFraction(float fraction)
    {
        if (fraction >= 0.f && fraction < 1.f)
        {
            sidebarWidthFraction = fraction;
            renderer.setViewportRect(makeViewportRect());
        }
    }
}