#include "Common.hlsl"

Texture2D<float4> EditorColourTexture : register(t0, space6);
Texture2D<float4> EditorDepthTexture : register(t1, space6);

struct RenderSpotLight
{
    float3 Position;
    float __pad0;
    float3 LightColour;
    float __pad1;
    float Intensity;
    float Radius;
};

cbuffer SpotLightBuffers : register(b0, space6)
{
    int SpotLightSize;
    int __pad0;
    int __pad1;
    int __pad2;
    RenderSpotLight SpotLights[32];
}

struct VertexOutput
{
	float4 Position : SV_POSITION;
	float2 UV : TEXCOORD0;
};

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

    if (SpotLightSize > 0)
    {
        for (int spotLightIdx = 0; spotLightIdx < SpotLightSize; spotLightIdx++)
        {
            const RenderSpotLight light = SpotLights[spotLightIdx];

            const float lightDistance = distance(
                float4(light.Position, 1.0f), 
                float4(worldPosition, 1.0f));

            if (lightDistance < light.Radius)
            {
                const float radius = lightDistance / light.Radius;
                const float attenuation = smoothstep(1.0, 0.0, radius);
                currentAlbedo += (albedo * attenuation) * light.Intensity;
            }
        }
    }
	return float4(ambientAlbedo + currentAlbedo, 1.0f);
}