// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include <Windows.h>

namespace DE
{
    class Window
    {
    public:
        Window(const wchar_t *title, int clientWidthPixels, int clientHeightPixels);
        ~Window();

        bool ProcessMessages();

    private:
        static LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
        LRESULT HandleMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

        HINSTANCE instanceHandle = nullptr;
        HWND windowHandle = nullptr;
        bool shouldClose = false;
    };
}