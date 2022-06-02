struct VertexInput
{
	float4 Pos : POSITION;
	float4 Normal : NORMAL;
	float4 Colour : COLOR;
	float2 UV : TEXCOORD;
};

struct VertexOutput
{
	float4 Pos : SV_POSITION;
	float4 Colour : COLOR0;
};


cbuffer ubo_camera : register(b0)
{
	float4x4 ProjView;
	float4x4 Projection;
	float4x4 View;
}

Texture2D GTexture : register(t1);
SamplerState GSampler : register(s1);

VertexOutput VSMain(const VertexInput input)
{
	VertexOutput vsOut;
	vsOut.Pos = mul(ProjView, float4(input.Pos.xyz, 1));
	vsOut.Colour = input.Colour;

	return vsOut;
}

float4 PSMain(VertexOutput input) : SV_TARGET
{	
	float4 shadow = GTexture.Sample(GSampler, float2(0, 0));
	return input.Colour;
}