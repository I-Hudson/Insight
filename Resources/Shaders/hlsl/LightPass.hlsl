#include "Common.hlsl"

Texture2D<float4> EditorColourTexture : register(t0, space6);
Texture2D<float4> EditorDepthTexture : register(t1, space6);

struct RenderSpotLight
{
    float4 Position;
    float4 LightColour;
    float Intensity;
    float Radius;
};

cbuffer SpotLightBuffers : register(b0, space6)
{
    int SpotLightSize;
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
	float3 DepthPosition = EditorDepthTexture.Sample(Clamp_Sampler, input.UV).xyz;
    float3 worldPosition = reconstruct_position(input.UV, DepthPosition.r, bf_Camera_Projection_View_Inverted);
	float3 albedo = (EditorColourTexture.Sample(Clamp_Sampler, input.UV).xyz) * 0.25f;

    for (int spotLightIdx = 0; spotLightIdx < SpotLightSize; ++spotLightIdx)
    {
        const float lightDistance = distance(
            float4(SpotLights[spotLightIdx].Position.xyz, 0.0f), 
            float4(worldPosition, 0.0f));

        if (lightDistance < SpotLights[spotLightIdx].Radius)
        {
        }
            albedo += float3(1.0f, 1.0f, 1.0f);
    }
    //albedo += float3(0.2f, 0.2f, 0.2f);
	return float4(albedo, 1.0f);
	//return float4(0.5, 1.0, 1.0, 1.0);
}