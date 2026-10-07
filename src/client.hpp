// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "renderer/renderer.hpp"
#include "window/window.hpp"
#include "sceneDescription.hpp"

namespace DE
{
    class Client
    {
    public:
        Client();

        void run();

        Handle addModel(const std::string &filePath);
        Handle addModelInstance(Handle modelHandle, Transform transform);

    private:
        Window window;
        Renderer renderer;

        SceneDescription sceneDescription;
        bool hasSceneChanged = true;

        HandleGenerator handleGenerator;
    };
}