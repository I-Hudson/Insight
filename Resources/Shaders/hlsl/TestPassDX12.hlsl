cbuffer UBO : register(b0)
{
	float4 ubo_Transform;
	float4 ubo_Colour;
	int ubo_Override;
}

//Texture2D<float> Texture : register(t0);
//SamplerState Sampler : register(s0);

struct VertexOutput
{
	float4 Position : SV_POSITION;
	float4 Colour : COLOR;
};

VertexOutput VSMain(VertexOutput input)
{
	VertexOutput o;
    o.Position 	= input.Position + ubo_Transform;
	o.Colour = input.Colour;
	return o;
}

float4 PSMain(VertexOutput input) : SV_TARGET
{
	if (ubo_Override == 0)
	{
		return input.Colour;
	}
	return ubo_Colour;
}