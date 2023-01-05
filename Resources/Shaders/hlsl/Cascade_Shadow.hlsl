#include "Common.hlsl"

struct VertexInput
{
	[[vk::location(0)]] float4 Pos : POSITION;
	[[vk::location(1)]] float4 Normal : NORMAL0;
	[[vk::location(2)]] float4 Colour : COLOR0;
	[[vk::location(3)]] float4 UV : TEXCOORD0;
};

struct VertexOutput
{
	float4 Pos : SV_POSITION;
};

struct Push_Constant
{
	float4x4 Transform;
	int Buffer_Light_Camera_Index;
};

[[vk::push_constant]] Push_Constant push_constant;

VertexOutput VSMain(const VertexInput input)
{
	VertexOutput vsOut;
	vsOut.Pos = float4(input.Pos.xyz, 1);

	vsOut.Pos = mul(push_constant.Transform, vsOut.Pos);
	vsOut.Pos = mul(bl_Camera_Proj_View[push_constant.Buffer_Light_Camera_Index], vsOut.Pos);

	return vsOut;
}