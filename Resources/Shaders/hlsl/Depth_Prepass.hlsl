#include "Common.hlsl"

struct VertexInput
{
	float4 Position : POSITION;
	float4 Normal : NORMAL0;
	float4 Colour : COLOR0;
	float4 UV : TEXCOORD0;
};

struct VertexOutput
{
	float4 Position : SV_POSITION;
};

cbuffer UBO : register(b0, PerObjectUniform)
{
	float4x4 ubo_Transform;
};

VertexOutput VSMain(const VertexInput input)
{
	VertexOutput vsOut;
	vsOut.Position = float4(input.Position.xyz, 1);

	vsOut.Position = mul(pubo_Transform, vsOut.Position);
	vsOut.Position = mul(Main_Camera_Proj_View, vsOut.Position);

	return vsOut;
}