#include "Common.hlsl"

RWTexture2D<float4> OutputTex  : register(u0);
Texture2D DepthTex : register(t0);
Texture2D ColourTex : register(t1);

Texture2DArray<float4> PointLightShadowMap[32] : register(t0, space1);

struct RenderPointLight
{
    float4x4 Projection;
    float4x4 View[6];
    float3 LightColour;
    float __pad1;
    float3 Position;
    float __pad2;

    float Intensity;
    float Radius;

    float __pad5_pTex1;
    float __pad5_pTex2;
};

cbuffer PointLightBuffers : register(b0, space1)
{
    RenderPointLight PointLights[32];
    int PointLightSize;
    float __pad0;
    float __pad1;
    float __pad2;
}

float PointShadowCalculation(Texture2DArray<float4> depthTexture, const float3 worldPosition, const RenderPointLight light)
{
    float3 wPosToLightPos = worldPosition - light.Position;
    const uint shadowSliceIndex = DirectionToCubeFaceIndex(wPosToLightPos);

    float shadowDepth = depthTexture[float3(wPosToLightPos.x, wPosToLightPos.y, shadowSliceIndex)].r;

    float4x4 shadowProjectionView = mul(light.Projection, light.View[shadowSliceIndex]);
    float3 shadowNDC = world_to_ndc(worldPosition, shadowProjectionView);

    if (shadowDepth < shadowNDC.z)
    {
        return 0;
    }
    return 1;
}

[numthreads(8, 8, 1)]
void CSMain(uint3 thread_id : SV_DispatchThreadID)
{
    float2 resolution_out;
    OutputTex.GetDimensions(resolution_out.x, resolution_out.y);
    if (any(int2(thread_id.xy) >= resolution_out))
    {
        return;
    }
    
	const float DepthValue = DepthTex[thread_id.xy].r;
    const float3 worldPosition = reconstruct_position(thread_id.xy / Render_Resolution, DepthValue, bf_Camera_Projection_View_Inverted);
	const float3 albedo = ColourTex[thread_id.xy].rgb;
    const float3 ambientAlbedo = albedo * 0.35;

    float3 currentAlbedo = float3(0, 0, 0);

    if (PointLightSize > 0)
    {
        for (int lightIdx = 0; lightIdx < PointLightSize; lightIdx++)
        {
            RenderPointLight light = PointLights[lightIdx];

            const float lightDistance = distance(
                float4(light.Position, 1.0f), 
                float4(worldPosition, 1.0f));

            if (lightDistance < light.Radius)
            {
                const float radius = lightDistance / light.Radius;
                const float attenuation = smoothstep(1.0, 0.0, radius);
                const float3 albedoLightColour = albedo * light.LightColour;
                const float3 albedoAttenuation = albedoLightColour * attenuation;

                const float shadow = PointShadowCalculation(PointLightShadowMap[lightIdx], worldPosition, light);
                currentAlbedo = shadow;//(albedoAttenuation * light.Intensity) * shadow;
                //currentAlbedo = shadow;
            }
        }
    }

    OutputTex[thread_id.xy] = float4(currentAlbedo, 1);
}