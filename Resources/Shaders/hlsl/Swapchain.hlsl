
float4 VSMain(uint id : SV_VertexID) : SV_POSITION
{
	float2 uv = float2((id << 1) & 2, id & 2);
	return float4(uv * float2(2, -2) + float2(-1, 1), 0, 1);
}

cbuffer ubo : register(b0)
{
	float4 SwapchainColour = float4(1, 0, 0, 1);
}

float4 PSMain() : SV_TARGET
{
	return float4(1, 0, 0, 1);
}