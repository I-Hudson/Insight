#include "PhysicsDebugBuffers.hlsl"

VertexOutput VSMain(const VertexInput input)
{
	VertexOutput vsOut;
	vsOut.Position = float4(input.Position.xyz, 1);
	vsOut.Position = mul(ProjectionView, vsOut.Position);
	vsOut.Colour = float4(input.Colour.xyz, 1.0f);
	vsOut.Tex = input.Tex;

	return vsOut;
}

struct PixelOutput
{
	float4 Colour	: SV_TARGET0;
};

PixelOutput PSMain(VertexOutput input)
{	
	PixelOutput Out;

	// Apply procedural pattern based on the uv coordinates
	bool2 less_half = input.Tex - floor(input.Tex) < float2(0.5, 0.5);
	float darken_factor = less_half.r ^ less_half.g? 0.5 : 1.0;

	// Fade out checkerboard pattern when it tiles too often
	float2 dx = ddx(input.Tex), dy = ddy(input.Tex);
	float texel_distance = sqrt(dot(dx, dx) + dot(dy, dy));
	darken_factor = lerp(darken_factor, 0.75, clamp(5.0 * texel_distance - 1.5, 0.0, 1.0));

	Out.Colour = darken_factor * input.Colour;
	return Out;
}