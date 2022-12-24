struct VertexOutput
{
	float4 Position : SV_POSITION;
	float4 Colour : COLOR0;
};

VertexOutput VSMain(VertexOutput input)
{
	VertexOutput o;
    o.Position 	= input.Position;
	o.Colour = input.Colour;
	return o;
}

float4 PSMain(VertexOutput input) : SV_TARGET
{
	return input.Colour;
}