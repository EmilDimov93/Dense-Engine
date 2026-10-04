// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

RaytracingAccelerationStructure Scene : register(t0);
RWTexture2D<float4> Output : register(u0);

struct RayPayload
{
    float3 color;
};

[shader("raygeneration")]
void RayGen()
{
    uint2 pixel = DispatchRaysIndex().xy;
    uint2 dims = DispatchRaysDimensions().xy;

    float2 ndc = (float2(pixel) + 0.5f) / float2(dims) * 2.0f - 1.0f;
    ndc.y = -ndc.y;

    RayDesc ray;
    ray.Origin = float3(0.0f, 0.0f, -2.0f);
    ray.Direction = normalize(float3(ndc, 1.0f));
    ray.TMin = 0.001f;
    ray.TMax = 1000.0f;

    RayPayload payload = { float3(0.0f, 0.0f, 0.0f) };
    TraceRay(Scene, RAY_FLAG_NONE, 0xFF, 0, 0, 0, ray, payload);

    Output[pixel] = float4(payload.color, 1.0f);
}

[shader("miss")]
void Miss(inout RayPayload payload)
{
    payload.color = float3(0.05f, 0.05f, 0.08f);
}

[shader("closesthit")]
void ClosestHit(inout RayPayload payload, BuiltInTriangleIntersectionAttributes attribs)
{
    payload.color = float3(1.0f - attribs.barycentrics.x - attribs.barycentrics.y, attribs.barycentrics.x, attribs.barycentrics.y);
}