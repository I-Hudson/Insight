#include "Common.hlsl"

struct VertexOutput
{
	float4 Pos : SV_POSITION;
};

cbuffer LightBuffer : register(b1, space0)
{
	float4x4 Light_ProjectionView;
};

cbuffer ObjectBuffer : register(b0, space1)
{
	float4x4 ubo_Transform;
	int ubo_SkinnedMesh;
};

VertexOutput VSMain(const GeoVertexInput input)
{
	VertexOutput vsOut;
	vsOut.Pos = float4(input.Pos, 1);

	[branch]
	if (ubo_SkinnedMesh == 1 && !GPUSkinningEnabled())
	{
		float4 worldNormal = float4(0,0,0,0);
		SkinMesh(input, vsOut.Pos, worldNormal);
	}

	vsOut.Pos = mul(ubo_Transform, vsOut.Pos);
	vsOut.Pos = mul(Light_ProjectionView, vsOut.Pos);

	return vsOut;
}