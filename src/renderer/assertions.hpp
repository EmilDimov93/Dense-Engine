// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "common.hpp"

#include <iostream>
#include <string_view>
#include <format>

namespace DE
{
    /**
     * @brief Fatal - Prints message and aborts
     * @param msg Message to print
     */
    [[noreturn]] inline void fatal(std::string_view msg)
    {
        std::cerr << msg << std::endl;

#ifdef _MSC_VER
        if (IsDebuggerPresent())
            __debugbreak();
#endif

        abort();
    }

    /**
     * @brief Warning - Prints warning message
     * @param msg Message to print
     */
    inline void warn(std::string_view msg)
    {
        std::cerr << msg << std::endl;
    }

    /**
     * @brief DirectX Assert - Prints message and aborts if DirectX function fails
     * @param result HRESULT to evaluate
     * @param msg Message to print on failure
     */
    inline void dxa(HRESULT result, std::string_view msg)
    {
        if (FAILED(result))
        {
            fatal(std::format("{} (HRESULT 0x{:08X})", msg, static_cast<uint32_t>(result)));
        }
    }

    /**
     * @brief Pointer Assert - Prints message and aborts if ComPtr is nullptr
     * @param comPtr ComPtr to evaluate
     * @param msg Message to print on failure
     */
    template <typename T>
    void ptra(const ComPtr<T> &comPtr, std::string_view msg)
    {
        if (!comPtr)
        {
            fatal(msg);
        }
    }

    /**
     * @brief Handle Assert - Prints message and aborts if HANDLE is nullptr
     * @param handle HANDLE to evaluate
     * @param msg Message to print on failure
     */
    inline void hndla(HANDLE handle, std::string_view msg)
    {
        if (handle == nullptr || handle == INVALID_HANDLE_VALUE)
        {
            fatal(msg);
        }
    }
}