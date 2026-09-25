// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "client.hpp"

namespace DE
{
    Client::Client() : window(L"Dense Engine", 500u, 500u), renderer(window.getHandle(), XMUINT2(window.getWidth(), window.getHeight())) {}

    void Client::run()
    {
        while(window.tick())
            renderer.tick();
    }
}