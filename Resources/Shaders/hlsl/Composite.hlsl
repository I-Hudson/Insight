
Texture2D GBufferColour : register(t0);
SamplerState GBufferColourSampler : register(s0);

Texture2D ShadowPass : register(t1);
SamplerState ShadowPassSampler : register(s1);

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
	float4 colour = GBufferColour.Sample(GBufferColourSampler, input.UV);
	float4 shadow = ShadowPass.Sample(ShadowPassSampler, input.UV);
	float4 result = float4(colour.xyz * shadow, 1);

	return result;
}