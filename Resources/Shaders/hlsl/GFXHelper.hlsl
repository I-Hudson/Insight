#include "Common.hlsl"

struct VertexInput
{
	float4 Position : POSITION;
	float4 Colour : COLOR0;
};

struct VertexOutput
{
	float4 Position 						: SV_POSITION;
	float4 Colour 					: COLOR0;
};

VertexOutput VSMain(const VertexInput input)
{
	VertexOutput vsOut;
	vsOut.Position = float4(input.Position.xyz, 1);
	vsOut.Colour = input.Colour;

	vsOut.Position = mul(bf_Camera_Proj_View, vsOut.Position);

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