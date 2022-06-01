
Texture2D GTexture : register(t0);
SamplerState GSampler : register(s0);

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
	float4 result = GTexture.Sample(GSampler, input.UV);
	return float4(result.xyz, 1);
}