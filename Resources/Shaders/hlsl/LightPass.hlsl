#include "Common.hlsl"

Texture2D DepthTexture : register(t0, space6);
Texture2D ColourTexture : register(t1, space6);
Texture2D WorldNormalTexture : register(t2, space6);

TextureCube PointLightShadowMap[32] : register(t0, space7);

#define DirectionLightCascadeCount 4
struct RenderDirectionalLight
{
    float4x4 ProjectionView[DirectionLightCascadeCount];
    float SplitDepth[DirectionLightCascadeCount];
    float3 LightDirection;
    float __rdlPad0;
    float3 LightColour;
    float __rdlPad1;

    float __rdlPad2_pTex1;
    float __rdlPad2_pTex2;
};

struct RenderPointLight
{
    float4x4 Projection;
    float4x4 View[6];
    float3 LightColour;
    float __rplPad1;
    float3 Position;
    float __rplPad2;

    float Intensity;
    float Radius;

    float __rplPad5_pTex1;
    float __rplPad5_pTex2;
};

cbuffer DirectionalLightBuffers : register(b0, space6)
{
    RenderDirectionalLight DirectionalLights[8];
    int DirectionalLightSize;
    float __dlbPad0;
    float __dlbPad1;
    float __dlbPad2;
}

cbuffer PointLightBuffers : register(b1, space6)
{
    RenderPointLight PointLights[32];
    int PointLightSize;
    float __plbPad0;
    float __plbPad1;
    float __plbPad2;
}

struct VertexOutput
{
	float4 Position : SV_POSITION;
	float2 UV : TEXCOORD0;
};

float3 LightPixel(const float3 colour, const float3 worldNormal, const float3 worldPosition, const RenderPointLight light)
{
    float3 diffuse = float3(0, 0, 0);

    float3 lightDirection = normalize(light.Position - worldPosition);
    float diffuseMultiplier = max(dot(lightDirection, worldNormal), 0.0);
    return colour * diffuseMultiplier;
}

float PointShadowCalculation(TextureCube depthTexture, const float3 worldPosition, const RenderPointLight light)
{
    float3 wPosToLightPos = worldPosition - light.Position;

    float shadowDepth = depthTexture.Sample(ClampToBoarder_Sampler, wPosToLightPos).r;

    const uint shadowSliceIndex = DirectionToCubeFaceIndex(wPosToLightPos);
    float4x4 shadowProjectionView = mul(light.Projection, light.View[shadowSliceIndex]);
    float3 shadowNDC = world_to_ndc(worldPosition, shadowProjectionView);

    [branch]
    if (shadowDepth < shadowNDC.z
        || shadowNDC.z > 1.0)
    {
        return 0;
    }
    return 1;
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
	const float DepthValue = DepthTexture.Sample(Clamp_Sampler, input.UV).r;
	const float3 albedo = (ColourTexture.Sample(Clamp_Sampler, input.UV).xyz);
	const float3 worldNormal = (WorldNormalTexture.Sample(Clamp_Sampler, input.UV).xyz);
    const float3 worldPosition = reconstruct_position(input.UV, DepthValue, bf_Camera_Projection_View_Inverted);
    const float3 ambientAlbedo = albedo * 0.4;

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

                float3 pixelColour = LightPixel(albedo, worldNormal, worldPosition, light);
                float3 pixelLightColour = pixelColour * light.LightColour;
                pixelColour = pixelLightColour * attenuation;

                const float shadow = PointShadowCalculation(PointLightShadowMap[lightIdx], worldPosition, light);
                currentAlbedo += (pixelColour * light.Intensity) * shadow;
            }
        }
    }
	return float4(ambientAlbedo + currentAlbedo, 1.0f);
}