#include "Common.hlsl"

struct VertexInput
{
	float4 Pos : POSITION;
	float4 Colour : COLOR0;
};

struct VertexOutput
{
	float4 Pos 						: SV_POSITION;
	float4 Colour 					: COLOR0;
};

VertexOutput VSMain(const VertexInput input)
{
	VertexOutput vsOut;
	vsOut.Pos = float4(input.Pos.xyz, 1);
	vsOut.Colour = input.Colour;

	vsOut.Pos = mul(bf_Camera_Proj_View, vsOut.Pos);

	return vsOut;
}

struct PixelOutput
{
	float4 Colour 			: SV_TARGET0;
};

PixelOutput PSMain(VertexOutput input)
{	
	PixelOutput Out;
	Out.Colour = input.Colour;
	return Out;
}