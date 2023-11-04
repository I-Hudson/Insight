#include "Common.hlsl"

struct VertexInput
{
	float4 Pos : POSITION;
	float4 Normal : NORMAL0;
	float4 Colour : COLOR0;
	float4 UV : TEXCOORD0;
};

struct VertexOutput
{
	float4 Pos : SV_POSITION;
};

cbuffer UBO : register(b1, PerObjectUniform)
{
	float4x4 ubo_Transform;
	int ubo_Buffer_Light_Camera_Index;
};


VertexOutput VSMain(const VertexInput input)
{
	VertexOutput vsOut;
	vsOut.Pos = float4(input.Pos.xyz, 1);

	vsOut.Pos = mul(ubo_Transform, vsOut.Pos);
	vsOut.Pos = mul(bl_Camera_Proj_View[ubo_Buffer_Light_Camera_Index], vsOut.Pos);

	return vsOut;
}