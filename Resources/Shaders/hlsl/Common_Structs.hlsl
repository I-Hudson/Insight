struct GeoVertexInput
{
	float3 Pos : POSITION;
	float3 Normal : NORMAL0;
	float3 Colour : COLOR0;
	float2 UV : TEXCOORD0;
};

struct GeoPixelInput
{
	float3 Position : SV_POSITION;
	float3 Normal : NORMAL0;
	float3 Colour : COLOR0;
	float2 UV : TEXCOORD0;
};