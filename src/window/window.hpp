// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include <windows.h>

namespace DE
{
    class Window
    {
    public:
        Window(const wchar_t *title, int width, int height);
        ~Window();

        bool tick();

        HWND getHandle() const { return windowHandle; }
        UINT getWidth() const { return width; }
        UINT getHeight() const { return height; }
        bool getMinimized() const { return isMinimized; }

    private:
        static LRESULT CALLBACK windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
        LRESULT handleMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

        HINSTANCE instanceHandle = nullptr;
        HWND windowHandle = nullptr;
        bool shouldClose = false;

        UINT width;
        UINT height;

        bool wasResized = false;
        bool isMinimized = false;
    };
}