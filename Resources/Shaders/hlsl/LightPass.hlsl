#include "Common.hlsl"

Texture2D<float4> EditorColourTexture : register(t0, space6);
Texture2D<float4> EditorDepthTexture : register(t1, space6);
TextureCube<float4> PointLightShadowMap[32] : register(t0, space7);

struct RenderPointLight
{
    float4x4 Projection;
    float4x4 View;
    float3 LightColour;
    float __pad1;

    float Intensity;
    float Radius;
    float FarPlane;
    float __pad2;

    float __pad3_pTex1;
    float __pad4_pTex2;
    float __pad5;
    float __pad6;
};

cbuffer PointLightBuffers : register(b0, space6)
{
    RenderPointLight PointLights[32];
    int PointLightSize;
}

struct VertexOutput
{
	float4 Position : SV_POSITION;
	float2 UV : TEXCOORD0;
};

float PointShadowCalculation(TextureCube<float4> depthTexture, const float3 worldPosition, const RenderPointLight light)
{
    float3 wPosToLightPos = worldPosition - light.View[3].xyz;
    float closestDepth = depthTexture.Sample(Clamp_Sampler, wPosToLightPos).r;
    closestDepth *= light.FarPlane;
    return closestDepth;
}

VertexOutput VSMain(uint id : SV_VertexID)
{
	VertexOutput o;
	o.UV = float2((id << 1) & 2, id & 2);
#ifdef VULKAN
    o.Position 	= float4(o.UV * 2.0f + -1.0f, 0.0f, 1.0f);
#elif DX12
    o.Position 	= float4(o.UV * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);
#endif
	return o;
}

float4 PSMain(VertexOutput input) : SV_TARGET
{
	const float DepthValue = EditorDepthTexture.Sample(Clamp_Sampler, input.UV).r;
    const float3 worldPosition = reconstruct_position(input.UV, DepthValue, bf_Camera_Projection_View_Inverted);
	const float3 albedo = (EditorColourTexture.Sample(Clamp_Sampler, input.UV).xyz);
    const float3 ambientAlbedo = albedo * 0.15;

    float3 currentAlbedo = float3(0, 0, 0);

    if (PointLightSize > 0)
    {
        for (int lightIdx = 0; lightIdx < PointLightSize; lightIdx++)
        {
            const RenderPointLight light = PointLights[lightIdx];
            float3 lightPosition = light.View[3].xyz;

            const float lightDistance = distance(
                float4(lightPosition, 1.0f), 
                float4(worldPosition, 1.0f));

            if (lightDistance < light.Radius)
            {
                const float radius = lightDistance / light.Radius;
                const float attenuation = smoothstep(1.0, 0.0, radius);
                const float3 albedoLightColour = albedo * light.LightColour;
                const float3 albedoAttenuation = albedoLightColour * attenuation;

                const float shadow = PointShadowCalculation(PointLightShadowMap[lightIdx], worldPosition, light);
                currentAlbedo += albedoAttenuation * light.Intensity;
                currentAlbedo = float3(shadow, shadow, shadow);
            }
        }
    }
	return float4(currentAlbedo, 1.0f);
}