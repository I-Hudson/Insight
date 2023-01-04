cbuffer UBO : register(b0, space0)
{
	float4 ubo_Transform;
	float4 ubo_Colour;
	int ubo_Override;
}

cbuffer BufferFrame : register(b0, space1)
{
	float4 bf_Colour;
}

Texture2D Texture : register(t0, space0);
SamplerState Sampler : register(s0, space2);

struct VertexOutput
{
	float4 Position : SV_POSITION;
	float4 Colour : COLOR0;
	float2 UV : TEXCOORD0;
};

VertexOutput VSMain(VertexOutput input)
{
	VertexOutput o;
    o.Position 	= input.Position + ubo_Transform;
	o.Colour = input.Colour;
	o.UV = input.UV;
	return o;
}

float4 PSMain(VertexOutput input) : SV_TARGET
{
	if (ubo_Override == 1)
	{
		return ubo_Colour;
	}
	else if (ubo_Override == 2)
	{
		return Texture.Sample(Sampler, input.UV);
	}
	return bf_Colour;
}