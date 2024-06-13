#include "PhysicsDebugBuffers.hlsl"

VertexOutput VSMain(const VertexInput input)
{
	VertexOutput vsOut;
	vsOut.Pos = float4(input.Pos.xyz, 1);
	vsOut.Colour = float4(input.Colour.xyz, 1.0f);

	vsOut.Pos = mul(ProjectionView, vsOut.Pos);

	return vsOut;
}

struct PixelOutput
{
	float4 Colour	: SV_TARGET0;
};

PixelOutput PSMain(VertexOutput input)
{	
	PixelOutput Out;
	Out.Colour = input.Colour;
	return Out;
}