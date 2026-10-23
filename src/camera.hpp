// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include "core/dxcommon.hpp"

#include <cstdint>

namespace DE
{
    struct CameraConstants
    {
        XMFLOAT3 position;
        float fov;
        XMFLOAT3 right;
        float orthoHalfHeight;
        XMFLOAT3 up;
        float padding;
        XMFLOAT3 forward;
    };

    struct Camera
    {
        XMFLOAT3 pos = {0.f, 0.f, 0.f};
        XMFLOAT3 rot = {0.f, 0.f, 0.f};

        float fov = 60;

        float moveSpeed = 5.f;
        float lookSensitivityRadPerPixel = 0.003f;

        CameraConstants toConstants() const
        {
            const XMMATRIX rotation = XMMatrixRotationRollPitchYaw(rot.x, rot.y, rot.z);

            CameraConstants constants{.position = pos, .fov = fov, .orthoHalfHeight = 2.f, .padding = 0.f};
            XMStoreFloat3(&constants.right, XMVector3TransformNormal(XMVectorSet(1.f, 0.f, 0.f, 0.f), rotation));
            XMStoreFloat3(&constants.up, XMVector3TransformNormal(XMVectorSet(0.f, 1.f, 0.f, 0.f), rotation));
            XMStoreFloat3(&constants.forward, XMVector3TransformNormal(XMVectorSet(0.f, 0.f, 1.f, 0.f), rotation));
            return constants;
        }
    };
}