// Copyright 2026 Emil Dimov
// Licensed under the Apache License, Version 2.0

// Bindings

RWTexture2D<float4> output : register(u0);

RaytracingAccelerationStructure scene : register(t0);

struct Material
{
    float4 diffuse;
    float metallic;
    float roughness;
    int textureIndex;
};
StructuredBuffer<Material> materials : register(t1);

struct Vertex
{
    float3 pos;
    float3 normal;
    float2 tex;
};
StructuredBuffer<Vertex> vertices : register(t2);
StructuredBuffer<uint> indices : register(t3);

struct TLASInstanceData
{
    uint vertexOffset;
    uint indexOffset;
    uint materialIndex;
};
StructuredBuffer<TLASInstanceData> tlasInstanceData : register(t4);

struct Settings
{
    int selectedID;
    float fov;
};

ConstantBuffer<Settings> settings : register(b0);

// Constants

static const uint MAX_BOUNCES = 1;

static const float3 clearColor = float3(0.5f, 0.5f, 0.8f);

// Payloads

struct RayPayload
{
    float3 color;
    uint depth;
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
    const float fovY = radians(settings.fov);
    const float tanHalfFov = tan(fovY / 2.f);

    ndc.x *= float(dims.x) / float(dims.y);
    ndc.y = -ndc.y;

    RayDesc ray;

    const float3 cameraPosition = float3(0.0f, 0.0f, -2.0f);
    
    if(settings.fov < 0.01f)
    {
        const float halfHeight = 2.f;
        ray.Origin = cameraPosition + float3(ndc * halfHeight, 0.0f);
        ray.Direction = float3(0.0f, 0.0f, 1.0f);
    }
    else
    {
        ray.Origin = cameraPosition;
        ray.Direction = normalize(float3(ndc * tanHalfFov, 1.0f));
    }

    ray.TMin = 0.001f;
    ray.TMax = 1000.0f;

    RayPayload payload = { float3(0.0f, 0.0f, 0.0f), 0 };
    TraceRay(scene, RAY_FLAG_NONE, 0xFF, 0, 0, 0, ray, payload);

    output[pixel] = float4(payload.color, 1.0f);
}

[shader("miss")]
void Miss(inout RayPayload payload)
{
    payload.color = clearColor;
}

[shader("miss")]
void ShadowMiss(inout ShadowPayload payload)
{
    payload.visibility = 1.0f;
}

[shader("closesthit")]
void ClosestHit(inout RayPayload payload, BuiltInTriangleIntersectionAttributes attribs)
{
    TLASInstanceData instanceData = tlasInstanceData[InstanceIndex()];

    Material hitMaterial = materials[instanceData.materialIndex];

    payload.color = hitMaterial.diffuse.rgb;

    float3 hitPos = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();

    if(hitMaterial.metallic > 0.f && payload.depth < MAX_BOUNCES)
    {
        uint baseIndex = instanceData.indexOffset + PrimitiveIndex() * 3;
        float3 vertex0Local = vertices[instanceData.vertexOffset + indices[baseIndex + 0]].pos;
        float3 vertex1Local = vertices[instanceData.vertexOffset + indices[baseIndex + 1]].pos;
        float3 vertex2Local = vertices[instanceData.vertexOffset + indices[baseIndex + 2]].pos;

        float3 faceNormalLocal = cross(vertex1Local - vertex0Local, vertex2Local - vertex0Local);
        float3 faceNormalWorld = normalize(mul(faceNormalLocal, (float3x3)WorldToObject3x4()));

        if (dot(faceNormalWorld, WorldRayDirection()) > 0.0f)
            faceNormalWorld = -faceNormalWorld;

        RayDesc reflectionRay;
        reflectionRay.Origin = hitPos - WorldRayDirection() * 0.001f;
        reflectionRay.Direction = reflect(WorldRayDirection(), faceNormalWorld);
        reflectionRay.TMin = 0.001f;
        reflectionRay.TMax = 1000.f * hitMaterial.metallic;

        RayPayload reflectionPayload = {clearColor, payload.depth + 1};
        TraceRay(scene, RAY_FLAG_NONE, 0xFF, 0, 0, 0, reflectionRay, reflectionPayload);

        float3 reflectedColor = reflectionPayload.color * hitMaterial.diffuse.rgb;
        payload.color = lerp(payload.color, reflectedColor, hitMaterial.metallic);
    }

    const float3 lightDir = normalize(float3(0.f, 1.f, 0.f));

    RayDesc shadowRay;
    shadowRay.Origin = hitPos - WorldRayDirection() * 0.001f;
    shadowRay.Direction = lightDir;
    shadowRay.TMin = 0.001f;
    shadowRay.TMax = 1000.0f;

    ShadowPayload shadowPayload = { 0.0f };
    TraceRay(scene, RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH | RAY_FLAG_SKIP_CLOSEST_HIT_SHADER, 0xFF, 0, 0, 1, shadowRay, shadowPayload);

    payload.color *= lerp(0.2f, 1.0f, shadowPayload.visibility);

    if(InstanceID() == settings.selectedID)
        payload.color += float3(0.3f, 0.3f, 0.f);
}