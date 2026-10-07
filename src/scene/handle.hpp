// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

namespace DE
{
    using Handle = uint64_t;

    class HandleGenerator
    {
    public:
        [[nodiscard]] static Handle generateHandle()
        {
            return Handle{++last};
        }

    private:
        static inline uint64_t last = 0;
    };
}