// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "renderer/renderer.hpp"
#include "window/window.hpp"
#include "scene/sceneDescription.hpp"
#include "ui/ui.hpp"
#include "core/rect.hpp"
#include "clientState.hpp"
#include "camera.hpp"

#include <chrono>

namespace DE
{
    class Client
    {
    public:
        Client(const char *sceneFilePath);
        ~Client();

        void run();

    private:
        Window window;
        Renderer renderer;

        SceneDescription sceneDescription;
        bool hasSceneChanged = true;

        HandleGenerator handleGenerator;

        UI ui;

        ClientState clientState;

        Camera camera;

        float sidebarWidthFraction = 0.25f;

        const std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();

        Handle addModel(const std::string &filePath);
        Handle addModelInstance(Handle modelHandle, Transform transform = {});

        Handle addPointLight(XMFLOAT3 pos, float intensity, XMFLOAT3 color, float range);
        Handle addDirectionalLight(XMFLOAT3 direction, float intensity, XMFLOAT3 color);

        void setSidebarWidthFraction(float fraction);

        Rect makeViewportRect() const;
    };
}