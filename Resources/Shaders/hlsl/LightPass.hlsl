#include "Common.hlsl"

#define DirectionLightCascadeCount 4
#define MaxDirectionLightCount 8
#define MaxPointLightCount 32


Texture2D DepthTexture : register(t0, space6);
Texture2D ColourTexture : register(t1, space6);
Texture2D WorldNormalTexture : register(t2, space6);

Texture2DArray DirectionalLightShadowMap[MaxDirectionLightCount] : register(t0, space7);
TextureCube PointLightShadowMap[MaxPointLightCount] : register(t32, space7);

struct RenderDirectionalLight
{
    float4x4 ProjectionView[DirectionLightCascadeCount + 1];
    float4 SplitDepth; // this is not an array as an array would take up 16 bytes per item https://stackoverflow.com/a/24311919
    float3 LightDirection;
    float __rdlPad0;
    float3 LightColour;
    float __rdlPad1;

    float __rdlPad2_pTex1;
    float __rdlPad2_pTex2;
    float __rdlPad3;
    float __rdlPad4;
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
    RenderDirectionalLight DirectionalLights[MaxDirectionLightCount];
    int DirectionalLightSize;
    float __dlbPad0;
    float __dlbPad1;
    float __dlbPad2;
}

cbuffer PointLightBuffers : register(b1, space6)
{
    RenderPointLight PointLights[MaxPointLightCount];
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

float3 LightPixel(const float3 colour, const float3 worldNormal, const float3 worldPosition, const float3 lightDirection)
{
    float3 diffuse = float3(0, 0, 0);

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

    //[unroll(MaxDirectionLightCount)]
    for (int directionalLightIdx = 0; directionalLightIdx < DirectionalLightSize; ++directionalLightIdx)
    {
        const RenderDirectionalLight light = DirectionalLights[directionalLightIdx];

        // select cascade layer
        const float4 fragPosViewSpace = mul(bf_Camera_View, float4(worldPosition, 1.0));
        const float depthViewSpace = abs(fragPosViewSpace.z);
        int cascadeLayer = DirectionLightCascadeCount - 1;
        //[unroll(DirectionLightCascadeCount)]
        for(int cascadeIndex = 0; cascadeIndex < DirectionLightCascadeCount; ++cascadeIndex)
        {
            const float cascadeDepth = abs(light.SplitDepth[cascadeIndex]);
            
            if(depthViewSpace < cascadeDepth)
            {
                cascadeLayer = cascadeIndex;
                break;
            }
        }

/*
        const float4 fragPosLightSpace = mul(light.ProjectionView[cascadeLayer], float4(worldPosition, 1.0));

        // perform perspective divide
        // perform perspective divide
        float3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
        // transform to [0,1] range
        projCoords = projCoords * 0.5 + 0.5;
        //return float4(projCoords.x, projCoords.y, projCoords.z, cascadeLayer);

        // transform to [0,1] range
       // float3 projCoords = world_to_ndc(worldPosition, light.ProjectionView[cascadeLayer]);
        //projCoords = remap_to_0_to_1(projCoords);
        //return float4(projCoords, cascadeLayer);

        // get depth of current fragment from light's perspective
        const float currentDepth = projCoords.z;
*/

        const float3 ndc = world_to_ndc(worldPosition, light.ProjectionView[cascadeLayer]);
        const float2 ndcUV = ndc_to_uv(ndc);
        const float currentDepth = ndc.z;
        return float4(ndc, cascadeLayer);

        [branch]
        if (currentDepth <= 1.0)
        {
            const float shadowDepth = DirectionalLightShadowMap[directionalLightIdx].Sample(ClampToBoarder_Sampler, float3(ndcUV, cascadeLayer)).r;
            const float shadow = shadowDepth < currentDepth|| currentDepth > 1.0 ? 0 : 1;

            const float3 pixelColour = LightPixel(albedo, worldNormal, worldPosition, light.LightDirection);
            currentAlbedo += pixelColour * shadow;
        }
    }

    [unroll(MaxPointLightCount)]
    for (int pointLightIdx = 0; pointLightIdx < PointLightSize; pointLightIdx++)
    {
        RenderPointLight light = PointLights[pointLightIdx];

        const float lightDistance = distance(
            float4(light.Position, 1.0f), 
            float4(worldPosition, 1.0f));

        if (lightDistance < light.Radius)
        {
            const float radius = lightDistance / light.Radius;
            const float attenuation = smoothstep(1.0, 0.0, radius);
            const float3 albedoLightColour = albedo * light.LightColour;
            const float3 albedoAttenuation = albedoLightColour * attenuation;

            float3 pixelColour = LightPixel(albedo, worldNormal, worldPosition, normalize(light.Position - worldPosition));
            float3 pixelLightColour = pixelColour * light.LightColour;
            pixelColour = pixelLightColour * attenuation;

            const float shadow = PointShadowCalculation(PointLightShadowMap[pointLightIdx], worldPosition, light);
            currentAlbedo += (pixelColour * light.Intensity) * shadow;
        }
    }
	return float4(ambientAlbedo + currentAlbedo, 1.0f);
}