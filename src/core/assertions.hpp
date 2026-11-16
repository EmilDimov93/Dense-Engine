// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "dxcommon.hpp"
#include "log.hpp"

#include <string_view>
#include <format>

namespace DE
{
    /**
     * @brief DirectX Assert - Prints message and aborts if DirectX function fails
     * @param result HRESULT to evaluate
     * @param msg Message to print on failure
     */
    inline void dxa(HRESULT result, std::string_view msg)
    {
        if (FAILED(result))
            log::fatal(std::format("{} (HRESULT 0x{:08X})", msg, static_cast<uint32_t>(result)));
    }

    /**
     * @brief Pointer Assert - Prints message and aborts if pointer is nullptr
     * @param ptr Pointer to evaluate
     * @param msg Message to print on failure
     */
    template <typename T>
    void ptra(const T &ptr, std::string_view msg)
    {
        if (!ptr)
            log::fatal(msg);
    }

    /**
     * @brief Handle Assert - Prints message and aborts if HANDLE is nullptr
     * @param handle HANDLE to evaluate
     * @param msg Message to print on failure
     */
    inline void hndla(HANDLE handle, std::string_view msg)
    {
        if (handle == nullptr || handle == INVALID_HANDLE_VALUE)
            log::fatal(msg);
    }
}