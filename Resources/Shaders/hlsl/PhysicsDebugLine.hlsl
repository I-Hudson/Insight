#include "PhysicsDebugBuffers.hlsl"

VertexOutput VSMain(const VertexInput input)
{
	VertexOutput vsOut;
	vsOut.Position = float4(input.Position.xyz, 1);
	vsOut.Colour = float4(input.Colour.xyz, 1.0f);

	vsOut.Position = mul(ProjectionView, vsOut.Position);

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