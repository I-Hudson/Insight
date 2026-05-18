#include "Common.hlsl"

GeoVertexOutput VSMain(const ShadowVertexInput input)
{
	GeoVertexOutput vsOut;
	vsOut.Position = float4(input.Position.xyz, 1);

	[branch]
	if (bpo_SkinnedMesh && !GPUSkinningEnabled())
	{
		float4 worldNormal = float4(0.0, 0.0, 0.0, 0.0);
		SkinMesh(input, vsOut.Position, worldNormal);
	}

	vsOut.Position = mul(bpo_Transform, vsOut.Position);
	vsOut.Position = mul(bf_Camera_Proj_View, vsOut.Position);

	return vsOut;
}