// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "client.hpp"

namespace DE
{
    Rect Client::makeViewportRect() const
    {
        XMUINT2 swapChainSize = renderer.getSwapChainSize();

        return Rect{{static_cast<uint32_t>(sidebarWidthFraction * swapChainSize.x), 0},
                    {static_cast<uint32_t>(swapChainSize.x - sidebarWidthFraction * swapChainSize.x), swapChainSize.y}};
    }

    Client::Client() : window(L"Dense Engine", 500u, 500u), renderer(window.getHandle(), XMUINT2(window.getWidth(), window.getHeight()))
    {}

    void Client::run()
    {
        while (window.tick())
        {
            if (window.getMinimized())
                continue;

            XMUINT2 swapChainSize = renderer.getSwapChainSize();
            XMUINT2 windowSize = XMUINT2(window.getWidth(), window.getHeight());

            if (swapChainSize.x != window.getWidth() || swapChainSize.y != window.getHeight())
            {
                renderer.resizeSwapChain(window.getWidth(), window.getHeight());
                renderer.setViewportRect(makeViewportRect());
            }

            if (hasSceneChanged)
            {
                renderer.uploadScene(sceneDescription);

                hasSceneChanged = false;
            }

            renderer.tick();
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
        if(fraction >= 0.f && fraction < 1.f)
        {
            sidebarWidthFraction = fraction;
            renderer.setViewportRect(makeViewportRect());
        }
    }
}