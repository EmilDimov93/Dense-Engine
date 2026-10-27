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
    float ior;
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

struct PointLight
{
    float3 pos;
    float intensity;
    float3 color;
    float range;
};
StructuredBuffer<PointLight> pointLights : register(t5);

struct DirectionalLight
{
    float3 direction;
    float intensity;
    float3 color;
};
StructuredBuffer<DirectionalLight> directionalLights : register(t6);

struct LightCounts
{
    uint pointLightCount;
    uint directionalLightCount;
};

ConstantBuffer<LightCounts> lightCounts : register(b0);

struct SceneSettings
{
    float3 clearColor;
    float exposure;
    float3 ambient;
};

ConstantBuffer<SceneSettings> sceneSettings : register(b1);

struct Camera
{
    float3 position;
    float fov;
    float3 right;
    float orthoHalfHeight;
    float3 up;
    float padding;
    float3 forward;
};

ConstantBuffer<Camera> camera : register(b2);

struct ClientState
{
    int selectedSceneComponentHandle;
    uint ssaaSamples;
    float time;
    bool lowerQuality;
};

ConstantBuffer<ClientState> clientState : register(b3);

// Constants

static const uint MAX_REFLECTION_BOUNCES = 1;
static const uint MAX_REFRACTION_DEPTH = 4;

static const float AIR_IOR = 1.0003f;

// Helpers

float3 linearToSRGB(float3 c)
{
    c = clamp(c, 0.f, 1.f);
    return select(c <= 0.0031308f, c * 12.92f, 1.055f * pow(c, 1.f / 2.4f) - 0.055f);
}

float3 toneMapACES(float3 x)
{
    const float a = 2.51f, b = 0.03f, c = 2.43f, d = 0.59f, e = 0.14f;
    return saturate((x * (a * x + b)) / (x * (c * x + d) + e));
}

// Payloads

struct RayPayload
{
    float4 color;
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

    uint ssaaSamples = (clientState.lowerQuality ? 1 : clientState.ssaaSamples);
    
    float3 color = {0.f, 0.f, 0.f};
    for(uint i = 0; i < ssaaSamples; i++)
    {
        uint side = (uint)sqrt((float)ssaaSamples);
        float2 offset = float2((i % side + 0.5f) / side - 0.5f, (i / side + 0.5f) / side - 0.5f);

        float2 ndc = (float2(pixel) + 0.5f + offset) / float2(dims) * 2.f - 1.f;
        const float fovY = radians(camera.fov);
        const float tanHalfFov = tan(fovY / 2.f);

        ndc.x *= float(dims.x) / float(dims.y);
        ndc.y = -ndc.y;

        RayDesc ray;
        
        if(camera.fov < 0.01f)
        {
            ray.Origin = camera.position + (camera.right * ndc.x + camera.up * ndc.y) * camera.orthoHalfHeight;
            ray.Direction = camera.forward;
        }
        else
        {
            ray.Origin = camera.position;
            ray.Direction = normalize(camera.forward + (camera.right * ndc.x + camera.up * ndc.y) * tanHalfFov);
        }

        ray.TMin = 0.001f;
        ray.TMax = 1000.f;

        RayPayload payload = { float4(0.f, 0.f, 0.f, 0.f), 0 };
        TraceRay(scene, RAY_FLAG_NONE, 0xFF, 0, 0, 0, ray, payload);

        color += payload.color.xyz;
    }

    color /= float(ssaaSamples);

    output[pixel] = float4(linearToSRGB(toneMapACES(color * sceneSettings.exposure)), 1.f);
}

[shader("miss")]
void Miss(inout RayPayload payload)
{
    payload.color = float4(sceneSettings.clearColor, 1.f);
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

    bool hitTriangleFrontSide = true;
    if (dot(faceNormalWorld, WorldRayDirection()) > 0.f)
    {
        faceNormalWorld = -faceNormalWorld;
        hitTriangleFrontSide = false;
    }

    float3 reflectedColor = sceneSettings.clearColor;
    if(payload.depth < MAX_REFLECTION_BOUNCES && !clientState.lowerQuality)
    {
        RayDesc reflectionRay;
        reflectionRay.Origin = hitPos - WorldRayDirection() * 0.001f;
        reflectionRay.Direction = reflect(WorldRayDirection(), faceNormalWorld);
        reflectionRay.TMin = 0.001f;
        reflectionRay.TMax = 1000.f;

        RayPayload reflectionPayload = {float4(sceneSettings.clearColor, 1.f), payload.depth + 1};
        TraceRay(scene, RAY_FLAG_NONE, 0xFF, 0, 0, 0, reflectionRay, reflectionPayload);

        reflectedColor = reflectionPayload.color.xyz;
    }

    float3 fresnel;
    {
        const float3 specularF0 = lerp(float3(0.04f, 0.04f, 0.04f), hitMaterial.diffuse.rgb, hitMaterial.metallic);

        const float cosViewNormal = clamp(dot(-WorldRayDirection(), faceNormalWorld), 0.f, 1.f);
        fresnel = specularF0 + (1.f - specularF0) * pow(1.f - cosViewNormal, 5.f);
    }

    float3 totalDirectLighting = float3(0.f, 0.f, 0.f);

    for(uint i = 0; i < lightCounts.directionalLightCount; i++)
    {
        const float3 lightDir = normalize(directionalLights[i].direction);

        RayDesc shadowRay;
        shadowRay.Origin = hitPos - WorldRayDirection() * 0.001f;
        shadowRay.Direction = lightDir;
        shadowRay.TMin = 0.001f;
        shadowRay.TMax = 1000.f;

        ShadowPayload shadowPayload = { 0.f };
        TraceRay(scene, RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH | RAY_FLAG_SKIP_CLOSEST_HIT_SHADER, 0xFF, 0, 0, 1, shadowRay, shadowPayload);

        const float cosLightNormal = clamp(dot(faceNormalWorld, lightDir), 0.f, 1.f);
        const float lightAmount = shadowPayload.visibility * cosLightNormal * directionalLights[i].intensity;

        totalDirectLighting += directionalLights[i].color * lightAmount;
    }

    for(uint i = 0; i < lightCounts.pointLightCount; i++)
    {
        const float3 origin = hitPos - WorldRayDirection() * 0.001f;

        const float3 lightOffset = pointLights[i].pos - origin;

        const float distance = length(lightOffset);

        if(distance > pointLights[i].range)
            continue;

        const float3 lightDir = lightOffset / distance;

        RayDesc shadowRay;
        shadowRay.Origin = origin;
        shadowRay.Direction = lightDir;
        shadowRay.TMin = 0.001f;
        shadowRay.TMax = distance - 0.001f;

        ShadowPayload shadowPayload = { 0.f };
        TraceRay(scene, RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH | RAY_FLAG_SKIP_CLOSEST_HIT_SHADER, 0xFF, 0, 0, 1, shadowRay, shadowPayload);

        const float cosLightNormal = clamp(dot(faceNormalWorld, lightDir), 0.f, 1.f);
        const float lightAmount = shadowPayload.visibility * cosLightNormal * pointLights[i].intensity / (distance * distance);

        totalDirectLighting += pointLights[i].color * lightAmount;
    }

    const float3 diffuseAlbedo = hitMaterial.diffuse.rgb * (1.f - hitMaterial.metallic);
    const float3 diffuseColor = (1.f - fresnel) * diffuseAlbedo * (sceneSettings.ambient + totalDirectLighting);

    const float smoothness = 1.f - hitMaterial.roughness;
    const float3 specularColor = fresnel * reflectedColor * smoothness * smoothness;

    payload.color = float4(diffuseColor + specularColor, 1.f);

    if(hitMaterial.diffuse.a < 1.f && payload.depth < MAX_REFRACTION_DEPTH && !clientState.lowerQuality)
    {
        RayDesc refractedRay;
        refractedRay.Origin = hitPos - faceNormalWorld * 0.001f;
        refractedRay.Direction = refract(WorldRayDirection(), faceNormalWorld, hitTriangleFrontSide ? AIR_IOR / hitMaterial.ior : hitMaterial.ior / AIR_IOR);
        if (dot(refractedRay.Direction, refractedRay.Direction) == 0.f)
        {
            refractedRay.Direction = reflect(WorldRayDirection(), faceNormalWorld);
            refractedRay.Origin = hitPos + faceNormalWorld * 0.001f;
        }
        refractedRay.TMin = 0.001f;
        refractedRay.TMax = 1000.f;

        RayPayload refractedPayload = {float4(sceneSettings.clearColor, 1.f), payload.depth + 1};
        TraceRay(scene, RAY_FLAG_NONE, 0xFF, 0, 0, 0, refractedRay, refractedPayload);

        payload.color.rgb = lerp(refractedPayload.color.rgb, payload.color.rgb, hitTriangleFrontSide ? hitMaterial.diffuse.a : 0.f);
    }

    if(InstanceID() == clientState.selectedSceneComponentHandle && payload.depth == 0)
        payload.color += float4(float3(0.8f, 0.8f, 0.f) * (1.f - clamp((abs(sin((hitPos.x * hitPos.z) * 2.f + clientState.time * 5.f)) + 0.2f), 0.f, 1.f)), 1.f);
}