// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "window.hpp"

namespace DE
{
    static const wchar_t *const WINDOW_CLASS_NAME = L"DenseWindowClass";

    Window::Window(const wchar_t *title, int width, int height) : width(width), height(height), instanceHandle(GetModuleHandleW(nullptr))
    {
        WNDCLASSEXW windowClass = {};
        windowClass.cbSize = sizeof(windowClass);
        windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        windowClass.lpfnWndProc = &Window::windowProc;
        windowClass.hInstance = instanceHandle;
        windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        windowClass.lpszClassName = WINDOW_CLASS_NAME;
        RegisterClassExW(&windowClass);

        const DWORD windowStyle = WS_OVERLAPPEDWINDOW;

        RECT windowRect = {0, 0, width, height};
        AdjustWindowRect(&windowRect, windowStyle, FALSE);

        windowHandle = CreateWindowExW(
            0,
            WINDOW_CLASS_NAME,
            title,
            windowStyle,
            CW_USEDEFAULT, CW_USEDEFAULT,
            windowRect.right - windowRect.left,
            windowRect.bottom - windowRect.top,
            nullptr, nullptr,
            instanceHandle,
            this);

        if (windowHandle == nullptr)
        {
            shouldClose = true;
            return;
        }

        ShowWindow(windowHandle, SW_SHOW);
    }

    Window::~Window()
    {
        if (windowHandle != nullptr)
            DestroyWindow(windowHandle);

        UnregisterClassW(WINDOW_CLASS_NAME, instanceHandle);
    }

    bool Window::tick()
    {
        MSG message = {};
        while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE))
        {
            if (message.message == WM_QUIT)
                shouldClose = true;

            TranslateMessage(&message);
            DispatchMessageW(&message);
        }

        return !shouldClose;
    }

    LRESULT CALLBACK Window::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        if (message == WM_NCCREATE)
        {
            const CREATESTRUCTW *createStruct = reinterpret_cast<CREATESTRUCTW *>(lParam);
            SetWindowLongPtrW(hwnd, GWLP_USERDATA,
                              reinterpret_cast<LONG_PTR>(createStruct->lpCreateParams));
        }

        Window *window = reinterpret_cast<Window *>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
        if (window != nullptr)
            return window->handleMessage(hwnd, message, wParam, lParam);

        return DefWindowProcW(hwnd, message, wParam, lParam);
    }

    LRESULT Window::handleMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        switch (message)
        {
        case WM_CLOSE:
            shouldClose = true;
            return 0;

        case WM_DESTROY:
            windowHandle = nullptr;
            PostQuitMessage(0);
            return 0;
        case WM_SIZE:
            isMinimized = (wParam == SIZE_MINIMIZED);
            if (isMinimized)
                return 0;
            const UINT newWidth = LOWORD(lParam);
            const UINT newHeight = HIWORD(lParam);
            if (newWidth != width || newHeight != height)
            {
                width = newWidth;
                height = newHeight;
            }
            return 0;
        }

        return DefWindowProcW(hwnd, message, wParam, lParam);
    }
}