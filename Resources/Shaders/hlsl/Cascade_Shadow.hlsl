#include "Common.hlsl"

struct VertexOutput
{
	float4 Position : SV_POSITION;
};

cbuffer UBO : register(b1, PerObjectUniform)
{
	float4x4 ubo_Transform;
	int ubo_Buffer_Light_Camera_Index;
};


VertexOutput VSMain(const GeoVertexInput input)
{
	VertexOutput vsOut;
	vsOut.Position = float4(input.Position, 1);

	vsOut.Position = mul(ubo_Transform, vsOut.Position);
	vsOut.Position = mul(bl_Camera_Proj_View[ubo_Buffer_Light_Camera_Index], vsOut.Position);

	return vsOut;
}