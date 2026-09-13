// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "client.hpp"

namespace DE
{
    Client::Client() : window(L"Dense Engine", 500, 500), renderer(window.getHandle(), XMUINT2(500, 500)) {}

    void Client::run()
    {
        while(window.tick())
        {
            
        }
    }
}