#define MAX_BONE_COUNT 4
struct GeoVertexInput
{
	float3 Pos : POSITION;
	float3 Normal : NORMAL0;
	float3 Colour : COLOR0;
	float2 UV : TEXCOORD0;

	int4 BoneIds : BLENDINDICES;
	float4 BoneWeights : BLENDWEIGHT;
};

struct GeoPixelInput
{
	float3 Position : SV_POSITION;
	float4 Normal : NORMAL0;
	float3 Colour : COLOR0;
	float2 UV : TEXCOORD0;
};