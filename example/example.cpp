// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#include "../src/client.hpp"

using namespace DE;

int main()
{
    Client client;

    Handle cube = client.addModel("../../example/assets/cube.obj");
    client.addModelInstance(cube, Transform({-2.f, 0.f, 6.f}, {0.5f, 0.5f, 0.f}));

    Handle cow = client.addModel("../../example/assets/cow.obj");
    client.addModelInstance(cow, Transform({2.f, -1.5f, 6.f}, {0.f, 3.14f, 0.f}));

    client.run();
}