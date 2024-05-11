#include "Common.hlsl"

Texture2D<float4> EditorColourTexture : register(t0, space6);
Texture2D<float4> EditorDepthTexture : register(t1, space6);
TextureCube<float4> PointLightShadowMap[32] : register(t0, space7);

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
    float FarPlane;
    float __pad3;

    float __pad4_pTex1;
    float __pad5_pTex2;
    float __pad6;
    float __pad7;
};

cbuffer PointLightBuffers : register(b0, space6)
{
    RenderPointLight PointLights[32];
    int PointLightSize;
    float CameraFarPlane;
}

struct VertexOutput
{
	float4 Position : SV_POSITION;
	float2 UV : TEXCOORD0;
};

float4 PointShadowCalculation(TextureCube<float4> depthTexture, const float3 worldPosition, const RenderPointLight light)
{
    float3 wPosToLightPos = worldPosition - light.Position;
    float3 wPosToLightNormal = normalize(wPosToLightPos);

    float4x4 lightProjView = mul(light.Projection, light.View);
    //float wPosInLightSpace =

    float lightDistance = length(worldPosition - light.View[3].xyz);

    // Map the light distance from 0-FarPlane to 0-1
    lightDistance = lightDistance / light.FarPlane;
    // Invert the distance from 0-1 to 1-0 so the close to the light you are the brighter it is.
    //lightDistance = 1.0 - lightDistance;
    
    float shadowDepth = depthTexture.Sample(Clamp_Sampler, wPosToLightNormal).r;
    float linearShadowDepth = LineariseFloat(shadowDepth, 0.1, light.FarPlane);
    linearShadowDepth /= light.FarPlane;
    float worldShadowDepth = linearShadowDepth * light.FarPlane;

    const uint shadowSliceIndex = DirectionToCubeFaceIndex(wPosToLightNormal);
    float4x4 shadowProjectionView = mul(light.Projection, light.View[shadowSliceIndex]);
    float3 shadowNDC = world_to_ndc(worldPosition, shadowProjectionView);

    float currentDepth = length(wPosToLightPos);
    // Map our current depth to 0-1
    float currentDepthClamp = currentDepth / CameraFarPlane;
    // now test for shadows
    float bias = 0.05; 
    float shadow = 0.0;
    
    if (shadowDepth > currentDepth)
    {
        shadow = 1.0; 
    }

    return float4(shadowNDC.z, shadowNDC.z, shadowNDC.z, shadowDepth);
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

    float4 currentAlbedo = float4(0, 0, 0, 0);

    if (PointLightSize > 0)
    {
        for (int lightIdx = 0; lightIdx < PointLightSize; lightIdx++)
        {
            RenderPointLight light = PointLights[lightIdx];
            //light.View = transpose(light.View);
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

                const float4 shadow = PointShadowCalculation(PointLightShadowMap[lightIdx], worldPosition, light);
                //currentAlbedo += albedoAttenuation * light.Intensity;
                currentAlbedo = shadow;
            }
        }
    }
	return currentAlbedo;//float4(, 1.0f);
}