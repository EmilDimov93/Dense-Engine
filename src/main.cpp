// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "client.hpp"

using namespace DE;

int main(int argc, char* argv[])
{
    Client client((argc > 1) ? argv[1] : nullptr);

    client.run();
}