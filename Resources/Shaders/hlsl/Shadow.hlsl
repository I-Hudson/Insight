#include "Common.hlsl"

struct VertexOutput
{
	float4 Pos : SV_POSITION;
};

cbuffer ubo_camera : register(b0)
{
	float4x4 ProjView;
	float4x4 Projection;
	float4x4 View;
}

VertexOutput VSMain(const GeoVertexInput input)
{
	VertexOutput vsOut;
	vsOut.Pos = mul(ProjView, float4(input.Pos.xyz, 1));
	return vsOut;
}