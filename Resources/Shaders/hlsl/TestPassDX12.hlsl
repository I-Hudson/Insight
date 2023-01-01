cbuffer UBO : register(b0)
{
	float4 ubo_Transform;
	float4 ubo_Colour;
	int ubo_Override;
}

Texture2D<float> Texture : register(t1);
SamplerState Sampler : register(s2);

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
		return Texture.Sample(Sampler, input.UV);
	if (ubo_Override == 1)
	{
		return ubo_Colour;
	}
	else if (ubo_Override == 2)
	{
	}
	return input.Colour;
}