[[vk::combinedImageSampler]][[vk::binding(0, 0)]]
Texture2D<float4> FullScreenTexture : register(t0);
[[vk::combinedImageSampler]][[vk::binding(0, 0)]]
SamplerState FullScreenSampler : register(s0);

struct VertexOutput
{
	float4 Position : SV_POSITION;
	float2 UV : TEXCOORD0;
};

VertexOutput VSMain(uint id : SV_VertexID)
{
	VertexOutput o;
	o.UV = float2((id << 1) & 2, id & 2);
	o.Position = float4(o.UV * float2(2, -2) + float2(-1, 1), 0, 1);
	return o;
}

float4 PSMain(VertexOutput input) : SV_TARGET
{	
	float4 result = FullScreenTexture.Sample(FullScreenSampler, input.UV);
	return float4(result.xyz, 1);
	//return float4(0.5, 1.0, 1.0, 1.0);
}