#include "Common.hlsl"

cbuffer BufferFrame : register(b0, space0)
{
	float4x4 bf_Camera_Proj_View;
	float4x4 bf_Camera_Projection;
	float4x4 bf_Camera_View;
	float4x4 bf_Camera_View_Inverted;
	float4x4 bf_Camera_Projection_View_Inverted;

    float2 Render_Resolution;
    float2 Output_Resolution;
    float Delta_Time;
}

cbuffer BufferPerObject : register(b0, space1)
{
    float4x4 bpo_Transform;
}

Texture2D Texture : register(t0, space1);
SamplerState Sampler : register(s0, space2);

GeoPixelInput VSMain(GeoVertexInput input)
{
	PixelInput o;

    o.Position = mul(bpo_Transform, input.Position);
    o.Position = mul(bf_Camera_Proj_View, o.Position);

	o.Normal = normalize(mul(bpo_Transform, float4(input.Normal.xyz, 0.0)));

	o.Colour = input.Colour;

	o.UV = input.UV.xy;

	return o;
}

float4 PSMain(GeoPixelInput input) : SV_TARGET
{
	return float4(input.Colour, 1.0);
}