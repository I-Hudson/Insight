Texture2D<float4> FullScreenTexture : register(t0, space0);
SamplerState FullScreenSampler : register(s0, space1);

struct VertexOutput
{
	float4 Position : SV_POSITION;
	float2 UV : TEXCOORD0;
};

VertexOutput VSMain(uint id : SV_VertexID)
{
	VertexOutput o;
	o.UV = float2((id << 1) & 2, id & 2);
#ifdef VULKAN
    o.Position 	= float4(o.UV * 2.0f + -1.0f, 0.0f, 1.0f);
#elif DX12
    o.Position 	= float4(o.UV * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);
#endif
	return o;
}

float4 PSMain(VertexOutput input) : SV_TARGET
{
	float4 result = FullScreenTexture.Sample(FullScreenSampler, input.UV);
	return float4(result.xyz, 1);
	//return float4(0.5, 1.0, 1.0, 1.0);
}