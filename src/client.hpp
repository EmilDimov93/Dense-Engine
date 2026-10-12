// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "renderer/renderer.hpp"
#include "window/window.hpp"
#include "sceneDescription.hpp"
#include "ui/ui.hpp"
#include "settings.hpp"
#include "rect.hpp"

namespace DE
{
    class Client
    {
    public:
        Client();
        ~Client();

        void run();

        Handle addModel(const std::string &filePath);
        Handle addModelInstance(Handle modelHandle, Transform transform);

        void setSidebarWidthFraction(float fraction);

    private:
        Window window;
        Renderer renderer;

        SceneDescription sceneDescription;
        bool hasSceneChanged = true;

        HandleGenerator handleGenerator;

        UI ui;

        Settings settings;

        float sidebarWidthFraction = 0.20f;

        Rect makeViewportRect() const;
    };
}