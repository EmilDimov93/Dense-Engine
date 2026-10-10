// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

RWTexture2D<float4> Output : register(u0);

RaytracingAccelerationStructure Scene : register(t0);

struct Material
{
    float4 diffuse;
    float metallic;
    float roughness;
    int textureIndex;
};
StructuredBuffer<Material> Materials : register(t1);

struct RayPayload
{
    float3 color;
};

struct ShadowPayload
{
    float visibility;
};

[shader("raygeneration")]
void RayGen()
{
    uint2 pixel = DispatchRaysIndex().xy;
    uint2 dims = DispatchRaysDimensions().xy;

    float2 ndc = (float2(pixel) + 0.5f) / float2(dims) * 2.0f - 1.0f;
    const float fovY = radians(60.0f);
    const float tanHalfFov = tan(fovY / 2.f);

    ndc.x *= float(dims.x) / float(dims.y);
    ndc *= tanHalfFov;
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

[shader("miss")]
void ShadowMiss(inout ShadowPayload payload)
{
    payload.visibility = 1.0f;
}

[shader("closesthit")]
void ClosestHit(inout RayPayload payload, BuiltInTriangleIntersectionAttributes attribs)
{
    Material material = Materials[InstanceID()];

    const float3 lightDir = normalize(float3(0.f, 1.f, 0.f));
    float3 hitPos = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();

    RayDesc shadowRay;
    shadowRay.Origin = hitPos - WorldRayDirection() * 0.001f;
    shadowRay.Direction = lightDir;
    shadowRay.TMin = 0.001f;
    shadowRay.TMax = 1000.0f;

    ShadowPayload shadow = { 0.0f };
    TraceRay(Scene, RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH | RAY_FLAG_SKIP_CLOSEST_HIT_SHADER, 0xFF, 0, 0, 1, shadowRay, shadow);

    payload.color = material.diffuse.rgb * lerp(0.2f, 1.0f, shadow.visibility);
}