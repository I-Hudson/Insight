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
	float4x4 Shadow_Camera_ProjView;
	float4x4 Shadow_Camera_Projection;
	float4x4 Shadow_Camera_View;
	float4 Shadow_Light_Direction;
	float2 Shadow_Resolution;
	float Shadow_CameraSplit_Depth;
	float pad0;
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
	vsOut.Pos = mul(Shadow_Camera_ProjView, vsOut.Pos);

	return vsOut;
}