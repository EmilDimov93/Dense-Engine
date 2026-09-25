// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "common.hpp"

#include <iostream>
#include <string_view>

namespace DE
{
    /**
    * @brief DirectX Assert - Prints message and aborts if DirectX function fails
    * @param result HRESULT to evaluate
    * @param msg Message to print on failure
    */
    inline void dxa(HRESULT result, std::string_view msg)
    {
        if(FAILED(result))
        {
            std::cerr << msg << std::endl;
            abort();
        }
    }

    /**
    * @brief Pointer Assert - Prints message and aborts if ComPtr is nullptr
    * @param comPtr ComPtr to evaluate
    * @param msg Message to print on failure
    */
    template<typename T>
    inline void ptra(const ComPtr<T> &comPtr, std::string_view msg)
    {
        if(!comPtr)
        {
            std::cerr << msg << std::endl;
            abort();
        }
    }

    /**
    * @brief Handle Assert - Prints message and aborts if HANDLE is nullptr
    * @param handle HANDLE to evaluate
    * @param msg Message to print on failure
    */
    inline void hndla(HANDLE handle, std::string_view msg)
    {
        if(handle == nullptr)
        {
            std::cerr << msg << std::endl;
            abort();
        }
    }
}