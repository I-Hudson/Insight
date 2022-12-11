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
	[[vk::location(0)]] float4 Pos 						: SV_POSITION;
	[[vk::location(1)]] float4 Colour 					: COLOR0;
	[[vk::location(2)]] float4 WorldPos 				: POSITION1;
	[[vk::location(3)]] float4 WorldNormal 				: NORMAL0;
	[[vk::location(4)]] float2 UV 						: TEXCOORD0;

	[[vk::location(5)]] float4 position_ss_current  	: SCREEN_POS;
    [[vk::location(6)]] float4 position_ss_previous 	: SCREEN_POS_PREVIOUS;
};

[[vk::binding(2, 1)]]
Texture2D<float4> Diffuse_Texture : register(t0);
[[vk::binding(3, 1)]]
Texture2D<float4> Normal_Texture : register(t1);
[[vk::binding(4, 1)]]
Texture2D<float4> Specular_Texture : register(t2);

VertexOutput VSMain(const VertexInput input)
{
	VertexOutput vsOut;
	vsOut.Pos = float4(input.Pos.xyz, 1);
	vsOut.Colour = input.Colour;

	vsOut.WorldPos = mul(bpo_Transform, vsOut.Pos);
	vsOut.Pos = mul(bf_Camera_Proj_View, vsOut.WorldPos);
	
	vsOut.WorldNormal = normalize(mul(bpo_Transform, float4(input.Normal.xyz, 0.0)));
	vsOut.UV = input.UV;

	vsOut.position_ss_current = mul(bf_Camera_View, vsOut.WorldPos);

	float4 world_pos_previous = mul(bpo_Previous_Transform, float4(input.Pos.xyz, 1));
	vsOut.position_ss_previous = mul(bf_Camera_View, world_pos_previous);

	return vsOut;
}

struct PixelOutput
{
	float4 Colour 			: SV_TARGET0;
	float4 World_Normal 	: SV_TARGET1;
	float2 Velocity 		: SV_TARGET2;
};

PixelOutput PSMain(VertexOutput input)
{	
    float2 position_uv_current  = ndc_to_uv(input.position_ss_current.xy / input.position_ss_current.w);
    float2 position_uv_previous = ndc_to_uv(input.position_ss_previous.xy / input.position_ss_previous.w);
    float2 velocity_uv          = position_uv_current - position_uv_previous;

	PixelOutput Out;
	if(bpo_Textures_Set[0] == 1)
	{
		Out.Colour = Diffuse_Texture.Sample(Reapt_Sampler, input.UV);
	}
	else
	{
		Out.Colour = float4(0.94, 0, 1, 1);
	}
	//Out.Colour = input.Colour;
	Out.World_Normal = float4(input.WorldNormal.xyz, 1.0);
	Out.Velocity = velocity_uv;

	return Out;
}