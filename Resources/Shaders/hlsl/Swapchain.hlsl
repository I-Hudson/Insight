
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

cbuffer ubo : register(b0)
{
	float2 SwapchainColour;
}
cbuffer ubo2 : register(b1)
{
	float2 SwapchainColour2;
}
cbuffer ubo_camera : register(b2)
{
	float4x4 ProjView;
	float4x4 Projection;
	float4x4 View;
}

//float4 VSMain(uint id : SV_VertexID) : SV_POSITION
VertexOutput VSMain(const VertexInput input)
{
	VertexOutput vsOut;
	//float2 uv = float2((id << 1) & 2, id & 2);
	//return float4(uv * float2(2, -2) + float2(-1, 1), 0, 1);
	vsOut.Pos = mul(ProjView, float4(input.Pos.xyz, 1));
	vsOut.Colour = input.Colour;

	return vsOut;
}

float4 PSMain(VertexOutput input) : SV_TARGET
{	
	float4 result = float4(SwapchainColour.x, SwapchainColour.y, SwapchainColour2.x, SwapchainColour2.y);
	//result += textureColor.Sample(samplerColor, float2(0, 0));
	return input.Colour;
	//return float4(1, 0, 0, 1);
}