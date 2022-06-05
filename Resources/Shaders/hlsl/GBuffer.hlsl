struct VertexInput
{
	float4 Pos : POSITION;
	float4 Normal : NORMAL;
	float4 Colour : COLOR;
	float2 UV : TEXCOORD;
};

struct VertexOutput
{
	float4 Pos : SV_POSITION;
	float4 Colour : COLOR0;
	float4 WorldPos : POSITION1;
	float4 ShadowWorldPos : POSITION2;
};


cbuffer ubo_camera : register(b0)
{
	float4x4 ProjView;
	float4x4 Projection;
	float4x4 View;
}

cbuffer shadow_camera : register(b1)
{
	float4x4 Shadow_ProjView;
	float4x4 Shadow_Projection;
	float4x4 Shadow_View;
}

Texture2D ShadowTexture : register(t2);
SamplerState ShadowSampler : register(s2);

VertexOutput VSMain(const VertexInput input)
{
	VertexOutput vsOut;
	vsOut.WorldPos = float4(input.Pos.xyz, 1);
	vsOut.Colour = input.Colour;
	vsOut.Pos = mul(ProjView, vsOut.WorldPos);
	vsOut.ShadowWorldPos = mul(Shadow_ProjView, vsOut.WorldPos);

	return vsOut;
}

float GetShadowValue(float4 shadowCoord, float2 offset)
{
	float3 shadowUV = shadowCoord.xyz / shadowCoord.w;
	float sampleValue = ShadowTexture.Sample(ShadowSampler, shadowUV.xy + offset).x;
	return sampleValue;
}

float4 PSMain(VertexOutput input) : SV_TARGET
{	
	float shadow = GetShadowValue(input.ShadowWorldPos, float2(0, 0));
	return float4(shadow, shadow, shadow, 1);
}