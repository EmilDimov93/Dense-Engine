// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include <cstdint>

namespace DE
{
    using Handle = uint64_t;

    class HandleGenerator
    {
    public:
        [[nodiscard]] Handle generateHandle()
        {
            return Handle{++last};
        }

        void setLast(Handle handle) { last = handle; }

    private:
        uint64_t last = 0;
    };
}