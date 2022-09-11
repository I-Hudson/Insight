#include "Common.hlsl"

struct VertexInput
{
	[[vk::location(0)]] float4 Pos : POSITION;
	[[vk::location(1)]] float4 Normal : NORMAL0;
	[[vk::location(2)]] float4 Colour : COLOR0;
	[[vk::location(3)]] float2 UV : TEXCOORD0;
};

struct VertexOutput
{
	float4 Pos : SV_POSITION;
	[[vk::location(0)]] float4 Colour : COLOR0;
	[[vk::location(1)]] float4 WorldPos : POSITION1;
	[[vk::location(2)]] float4 ShadowWorldPos : POSITION2;
	[[vk::location(3)]] float4 WorldNormal : NORMAL0;
	[[vk::location(4)]] float2 UV : TEXCOORD0;

	[[vk::location(5)]] float4 position_ss_current  : SCREEN_POS;
    [[vk::location(6)]] float4 position_ss_previous : SCREEN_POS_PREVIOUS;
    [[vk::location(7)]] float4 position_screen_space : POSITION3;
};

struct PushConstant
{
	float4x4 Transform;
};
[[vk::push_constant]] PushConstant push_constant;

VertexOutput VSMain(const VertexInput input)
{
	VertexOutput vsOut;
	vsOut.WorldPos = float4(input.Pos.xyz, 1);
	vsOut.Colour = input.Colour;

	vsOut.WorldPos = mul(push_constant.Transform, vsOut.WorldPos);
	vsOut.Pos = mul(Main_Camera_Proj_View, vsOut.WorldPos);
	vsOut.position_screen_space = mul(Main_Camera_View, vsOut.WorldPos);

	vsOut.WorldNormal = normalize(mul(push_constant.Transform, float4(input.Normal.xyz, 0.0)));
	vsOut.UV = input.UV;

	return vsOut;
}

struct PixelOutput
{
	float4 Colour : SV_TARGET0;
	float4 World_Normal : SV_TARGET1;
	float2 Velocity : SV_TARGET2;
};

PixelOutput PSMain(VertexOutput input)
{	
    float2 position_uv_current  = ndc_to_uv(input.position_ss_current.xy / input.position_ss_current.w);
    float2 position_uv_previous = ndc_to_uv(input.position_ss_previous.xy / input.position_ss_previous.w);
    float2 velocity_uv          = position_uv_current - position_uv_previous;

	PixelOutput Out;
	Out.Colour = input.Colour;
	Out.World_Normal = float4(input.WorldNormal.xyz, 1.0);
	Out.Velocity = float2(0, 0); // velocity_uv

	return Out;
}