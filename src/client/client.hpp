// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "renderer/renderer.hpp"
#include "window/window.hpp"

namespace DE
{
    class Client
    {
    public:
        Client();

        void run();

    private:
        Window window;
        Renderer renderer;
    };
}