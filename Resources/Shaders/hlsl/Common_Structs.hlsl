#define MAX_BONE_COUNT 4
struct GeoVertexInput
{
	float3 Pos : POSITION;
	float3 Normal : NORMAL0;
	float3 Colour : COLOR0;
	float2 UV : TEXCOORD0;

	uint4 BoneIds : TEXCOORD1;
	float4 BoneWeights : TEXCOORD2;
};

struct GeoPixelInput
{
	float3 Position : SV_POSITION;
	float3 Normal : NORMAL0;
	float3 Colour : COLOR0;
	float2 UV : TEXCOORD0;
};