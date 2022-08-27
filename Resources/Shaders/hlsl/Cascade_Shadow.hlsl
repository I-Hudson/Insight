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
};

[[vk::binding(0, 0)]]
cbuffer ubo_camera : register(b0)
{
	float4x4 ProjView;
	float4x4 Projection;
	float4x4 View;
	float DpethSplit;
}

struct PushConstant
{
	float4x4 Transform;
};
[[vk::push_constant]] PushConstant push_constant;

VertexOutput VSMain(const VertexInput input)
{
	VertexOutput vsOut;
	vsOut.Pos = float4(input.Pos.xyz, 1);

	vsOut.Pos = mul(push_constant.Transform, vsOut.Pos);
	vsOut.Pos = mul(ProjView, vsOut.Pos);

	return vsOut;
}