// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "client.hpp"

namespace DE
{
    Client::Client() : window(L"Dense Engine", 500u, 500u), renderer(window.getHandle(), XMUINT2(window.getWidth(), window.getHeight())) {}

    void Client::run()
    {
        while (window.tick())
        {
            if(window.getMinimized())
                continue;

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
}