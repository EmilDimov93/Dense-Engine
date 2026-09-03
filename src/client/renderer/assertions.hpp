// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>
#include <cstdint>
#include <string>
#include <iostream>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

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
            std::cout << msg << std::endl;
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
            std::cout << msg << std::endl;
            abort();
        }
    }
}