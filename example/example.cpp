// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "../src/client.hpp"

using namespace DE;

int main()
{
    Client client;

    Handle cube = client.addModel("../../example/assets/cube.obj");
    client.addModelInstance(cube, Transform({0.f, 0.f, 3.f}, {0.5f, 0.5f, 0.f}));

    client.setSidebarWidthFraction(0.5f);

    client.run();
}