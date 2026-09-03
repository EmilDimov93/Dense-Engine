// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "../src/client/window/window.hpp"
#include "../src/client/renderer/renderer.hpp"

int main()
{
    DE::Renderer r;

    DE::Window window(L"Example", 500, 500);

    while (window.ProcessMessages())
    {
        
    }
}