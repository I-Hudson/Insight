
struct VertexInput
{
	float4 Pos : POSITION;
	//float4 Normal : NORMAL;
	//float4 Colour : COLOR;
	//float2 UV : TEXCOORD;
};

//float4 VSMain(uint id : SV_VertexID) : SV_POSITION
float4 VSMain(const VertexInput input) : SV_POSITION
{
	//float2 uv = float2((id << 1) & 2, id & 2);
	//return float4(uv * float2(2, -2) + float2(-1, 1), 0, 1);

	return float4(input.Pos.xyz, 1);
}

cbuffer ubo : register(b0)
{
	float2 SwapchainColour;
}
cbuffer ubo2 : register(b1)
{
	float2 SwapchainColour2;
}

float4 PSMain() : SV_TARGET
{	
	float4 result = float4(SwapchainColour.x, SwapchainColour.y, SwapchainColour2.x, SwapchainColour2.y);
	//result += textureColor.Sample(samplerColor, float2(0, 0));
	return result;
	//return float4(1, 0, 0, 1);
}