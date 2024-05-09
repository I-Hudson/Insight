#include "Common_Structs.hlsl"

struct VertexOutput
{
	float4 Pos : SV_POSITION;
};

cbuffer LightBuffer : register(b0, space0)
{
	float4x4 Light_ProjectionView;
};

cbuffer ObjectBuffer : register(b0, space1)
{
	float4x4 ubo_Transform;
};

VertexOutput VSMain(const GeoVertexInput input)
{
	VertexOutput vsOut;
	vsOut.Pos = float4(input.Pos, 1);

	vsOut.Pos = mul(ubo_Transform, vsOut.Pos);
	vsOut.Pos = mul(Light_ProjectionView, vsOut.Pos);

	return vsOut;
}