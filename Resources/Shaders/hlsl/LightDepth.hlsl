#include "Common.hlsl"

struct VertexOutput
{
	float4 Position : SV_POSITION;
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

VertexOutput VSMain(const ShadowVertexInput input)
{
	const float4x4 modelSpaceTransform = ubo_Transform;
	const float4x4 lightProjectionView = Light_ProjectionView;

	VertexOutput vsOut;
	vsOut.Position = float4(input.Position, 1);

	[branch]
	if (ubo_SkinnedMesh == 1 && !GPUSkinningEnabled())
	{
		float4 worldNormal = float4(0,0,0,0);
		SkinMesh(input, vsOut.Position, worldNormal);
	}

	vsOut.Position = mul(modelSpaceTransform, vsOut.Position);
	vsOut.Position = mul(lightProjectionView, vsOut.Position);

	return vsOut;
}