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

    float2 ndc = (float2(pixel) + 0.5f) / float2(dims) * 2.f - 1.f;
    const float fovY = radians(settings.fov);
    const float tanHalfFov = tan(fovY / 2.f);

    ndc.x *= float(dims.x) / float(dims.y);
    ndc.y = -ndc.y;

    RayDesc ray;

    const float3 cameraPosition = float3(0.f, 0.f, -2.f);
    
    if(settings.fov < 0.01f)
    {
        const float halfHeight = 2.f;
        ray.Origin = cameraPosition + float3(ndc * halfHeight, 0.f);
        ray.Direction = float3(0.f, 0.f, 1.f);
    }
    else
    {
        ray.Origin = cameraPosition;
        ray.Direction = normalize(float3(ndc * tanHalfFov, 1.f));
    }

    ray.TMin = 0.001f;
    ray.TMax = 1000.f;

    RayPayload payload = { float3(0.f, 0.f, 0.f), 0 };
    TraceRay(scene, RAY_FLAG_NONE, 0xFF, 0, 0, 0, ray, payload);

    output[pixel] = float4(payload.color, 1.f);
}

[shader("miss")]
void Miss(inout RayPayload payload)
{
    payload.color = clearColor;
}

[shader("miss")]
void ShadowMiss(inout ShadowPayload payload)
{
    payload.visibility = 1.f;
}

[shader("closesthit")]
void ClosestHit(inout RayPayload payload, BuiltInTriangleIntersectionAttributes attribs)
{
    TLASInstanceData instanceData = tlasInstanceData[InstanceIndex()];

    Material hitMaterial = materials[instanceData.materialIndex];

    float3 faceNormalWorld;
    {
        uint baseIndex = instanceData.indexOffset + PrimitiveIndex() * 3;
        float3 vertex0NormalLocal = vertices[instanceData.vertexOffset + indices[baseIndex + 0]].normal;
        float3 vertex1NormalLocal = vertices[instanceData.vertexOffset + indices[baseIndex + 1]].normal;
        float3 vertex2NormalLocal = vertices[instanceData.vertexOffset + indices[baseIndex + 2]].normal;

        const float3 barycentricWeights = float3(1.f - attribs.barycentrics.x - attribs.barycentrics.y, attribs.barycentrics.x, attribs.barycentrics.y);

        float3 faceNormalLocal = vertex0NormalLocal * barycentricWeights.x + 
                                    vertex1NormalLocal * barycentricWeights.y + 
                                    vertex2NormalLocal * barycentricWeights.z;
        faceNormalWorld = normalize(mul(faceNormalLocal, (float3x3)WorldToObject3x4()));
    }

    float3 hitPos = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();

    if (dot(faceNormalWorld, WorldRayDirection()) > 0.f)
        faceNormalWorld = -faceNormalWorld;

    float3 reflectedColor = clearColor;
    if(payload.depth < MAX_BOUNCES)
    {
        RayDesc reflectionRay;
        reflectionRay.Origin = hitPos - WorldRayDirection() * 0.001f;
        reflectionRay.Direction = reflect(WorldRayDirection(), faceNormalWorld);
        reflectionRay.TMin = 0.001f;
        reflectionRay.TMax = 1000.f;

        RayPayload reflectionPayload = {clearColor, payload.depth + 1};
        TraceRay(scene, RAY_FLAG_NONE, 0xFF, 0, 0, 0, reflectionRay, reflectionPayload);

        reflectedColor = reflectionPayload.color;
    }

    float3 fresnel;
    {
        const float3 specularF0 = lerp(float3(0.04f, 0.04f, 0.04f), hitMaterial.diffuse.rgb, hitMaterial.metallic);

        const float cosViewNormal = clamp(dot(-WorldRayDirection(), faceNormalWorld), 0.f, 1.f);
        fresnel = specularF0 + (1.f - specularF0) * pow(1.f - cosViewNormal, 5.f);
    }

    const float3 lightDir = normalize(float3(0.f, 1.f, 0.f));

    RayDesc shadowRay;
    shadowRay.Origin = hitPos - WorldRayDirection() * 0.001f;
    shadowRay.Direction = lightDir;
    shadowRay.TMin = 0.001f;
    shadowRay.TMax = 1000.f;

    ShadowPayload shadowPayload = { 0.f };
    TraceRay(scene, RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH | RAY_FLAG_SKIP_CLOSEST_HIT_SHADER, 0xFF, 0, 0, 1, shadowRay, shadowPayload);

    const float3 specularColor = fresnel * reflectedColor;

    const float cosLightNormal = clamp(dot(faceNormalWorld, lightDir), 0.f, 1.f);
    const float directLighting = lerp(0.2f, 1.f, shadowPayload.visibility * cosLightNormal);
    const float3 diffuseAlbedo = hitMaterial.diffuse.rgb * (1.f - hitMaterial.metallic);
    const float3 diffuseColor = (1.f - fresnel) * diffuseAlbedo * directLighting;

    payload.color = diffuseColor + specularColor;

    if(InstanceID() == settings.selectedID && payload.depth == 0)
        payload.color += float3(0.3f, 0.3f, 0.f);
}