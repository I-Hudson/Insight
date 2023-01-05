
struct VertexInput
{
	float4 Pos : POSITION;
	float4 Normal : NORMAL;
	float4 Colour : COLOR;
	float4 UV : TEXCOORD;
};

struct VertexOutput
{
	float4 Pos : SV_POSITION;
};

cbuffer ubo_camera : register(b0)
{
	float4x4 ProjView;
	float4x4 Projection;
	float4x4 View;
}

VertexOutput VSMain(const VertexInput input)
{
	VertexOutput vsOut;
	vsOut.Pos = mul(ProjView, float4(input.Pos.xyz, 1));
	return vsOut;
}