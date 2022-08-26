struct VertexInput
{
	[[vk::location(0)]] float4 Pos : POSITION;
	[[vk::location(1)]] float4 Normal : NORMAL0;
	[[vk::location(2)]] float4 Colour : COLOR0;
	[[vk::location(3)]] float2 UV : TEXCOORD0;
};

struct VertexOutput
{
	float4 Pos : SV_POSITION;
	[[vk::location(0)]] float4 Colour : COLOR0;
	[[vk::location(1)]] float4 WorldPos : POSITION1;
	[[vk::location(2)]] float4 ShadowWorldPos : POSITION2;
	[[vk::location(3)]] float3 WorldNormal : NORMAL0;
	[[vk::location(4)]] float2 UV : TEXCOORD0;
};

[[vk::binding(0, 0)]]
cbuffer ubo_camera : register(b0)
{
	float4x4 ProjView;
	float4x4 Projection;
	float4x4 View;
}
[[vk::binding(1, 0)]]
cbuffer shadow_camera : register(b1)
{
	float4x4 Shadow_ProjView;
	float4x4 Shadow_Projection;
	float4x4 Shadow_View;
	float2 Shadow_TextureSize;
}

[[vk::combinedImageSampler]][[vk::binding(2, 0)]]
Texture2D<float4> ShadowTexture : register(t0);
[[vk::combinedImageSampler]][[vk::binding(2, 0)]]
SamplerState ShadowSampler : register(s0);

VertexOutput VSMain(const VertexInput input)
{
	VertexOutput vsOut;
	vsOut.WorldPos = float4(input.Pos.xyz, 1);
	vsOut.Colour = input.Colour;
	vsOut.Pos = mul(ProjView, vsOut.WorldPos);

	// Invert our view matrix as we invert the view port. 
	//float4x4 shadowView = Shadow_View;
	//shadowView[1][1] *= -1;
	//float4x4 shadowProjectionView = mul(Shadow_Projection, shadowView);
	//vsOut.ShadowWorldPos = mul(shadowProjectionView, vsOut.WorldPos);

	vsOut.WorldNormal = input.Normal.xyz;
	vsOut.UV = input.UV;

	return vsOut;
}

float4 GetShadowCoord(float4 shadowCoord)
{
	float4 shadowUV = shadowCoord / shadowCoord.w;
	//shadowUV.x = (shadowUV.x + 1.0) * 0.5;
	//shadowUV.y = (shadowUV.y + 1.0) * 0.5;
	return shadowUV;
}

float GetShadowValue(float4 shadowCoord, float2 offset)
{
	float sampleValue = ShadowTexture.Sample(ShadowSampler, shadowCoord.xy + offset).r;
	return sampleValue;
}

float linearize_depth(float d, float zNear, float zFar)
{
	return (zNear * zFar / (zFar + d * (zNear - zFar))) / zFar;
}

float LinearizeDepth(float depth, float near_plane, float far_plane)
{
	float z = depth;// * 2.0 - 1.0; // Back to NDC 
	return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));
}

struct PixelOutput
{
	float4 Colour : SV_TARGET0;
	float4 WorldPosition : SV_TARGET1;
};

PixelOutput PSMain(VertexOutput input)
{	
	//float4 shadowUV = GetShadowCoord(input.ShadowWorldPos);
	//float shadow = GetShadowValue(shadowUV, float2(0, 0));

	//shadow = linearize_depth(shadow, 1.0, 512.0);
	//shadow = LinearizeDepth(shadow, 1.0, 512.0);
	//return float4(shadowUV.x, shadowUV.y, shadowUV.z, 1);

	PixelOutput Out;
	Out.Colour = input.Colour;
	Out.WorldPosition = input.WorldPos;
	return Out;
}