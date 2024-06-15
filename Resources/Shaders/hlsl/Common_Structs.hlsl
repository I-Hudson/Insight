#define MAX_BONE_COUNT 4
struct GeoVertexInput
{
	float3 Pos : POSITION;
	float3 Normal : NORMAL0;
#ifdef VERTEX_COLOUR_PACKED
	int Colour : COLOR0;
#else
	float3 Colour : COLOR0;
#endif
	float2 UV : TEXCOORD0;
#ifdef BONE_ID_PACKED
	int BoneIds : BLENDINDICES;
#else
	int4 BoneIds : BLENDINDICES;
#endif
	float4 BoneWeights : BLENDWEIGHT;
};

#ifdef VERTEX_COLOUR_PACKED
float4 GetVertexColour(const in GeoVertexInput vertex)
{
	const float r = vertex.Colour / 255;
	const float g = (vertex.Colour >> 8) / 255;
	const float b = (vertex.Colour >> 16) / 255;
	return float4(r, g, b, 1.0);
}
#else
float4 GetVertexColour(const in GeoVertexInput vertex)
{
	return float4(vertex.Colour, 1.0);
}
#endif

#ifdef BONE_ID_PACKED
int GetVertexBondId(const in int boneIds, const in int boneIdx)
{
	const int bitShiftStep = 8;
	const int bitShift = bitShiftStep * boneIdx;
	const int boneId = (boneIds >> bitShift) & 0xFF;
	return boneId;
}
#else
int GetVertexBondId(const in int4 boneIds, const in int boneIdx)
{
	return boneIds[boneIdx];
}
#endif

struct GeoPixelInput
{
	float3 Position : SV_POSITION;
	float4 Normal : NORMAL0;
	float3 Colour : COLOR0;
	float2 UV : TEXCOORD0;
};