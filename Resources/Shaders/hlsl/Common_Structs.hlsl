#define MAX_BONE_COUNT 4
struct GeoVertexInput
{
	float3 Pos : POSITION;
#ifdef VERTEX_NORMAL_PACKED
	int Normal : NORMAL0;
#else
	float3 Normal : NORMAL0;
#endif
#ifdef VERTEX_COLOUR_PACKED
	int Colour : COLOR0;
#else
	float3 Colour : COLOR0;
#endif
	float2 UV : TEXCOORD0;
#ifdef VERTEX_BONE_ID_PACKED
	int BoneIds : BLENDINDICES;
#else
	int4 BoneIds : BLENDINDICES;
#endif
#ifdef VERTEX_BONE_WEIGHT_PACKED
	int2 BoneWeights : BLENDWEIGHT;
#else
	float4 BoneWeights : BLENDWEIGHT;
#endif
};

#ifdef VERTEX_NORMAL_PACKED
float4 GetVertexNormal(const in GeoVertexInput vertex)
{
	const float x = (float)(vertex.Normal & 0xFF) / 255.0;
	const float y = (float)((vertex.Normal >> 8) & 0xFF) / 255.0;
	const float z = (float)((vertex.Normal >> 16) & 0xFF) / 255.0;
	return float4(x, y, z, 0.0);
}
#else
float4 GetVertexNormal(const in GeoVertexInput vertex)
{
	return float4(vertex.Normal, 1.0);
}
#endif

#ifdef VERTEX_COLOUR_PACKED
float4 GetVertexColour(const in GeoVertexInput vertex)
{
	const float r = (float)(vertex.Colour & 0xFF) / 255.0;
	const float g = (float)((vertex.Colour >> 8) & 0xFF) / 255.0;
	const float b = (float)((vertex.Colour >> 16) & 0xFF) / 255.0;
	return float4(r, g, b, 1.0);
}
#else
float4 GetVertexColour(const in GeoVertexInput vertex)
{
	return float4(vertex.Colour, 1.0);
}
#endif

#ifdef VERTEX_BONE_ID_PACKED
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

#ifdef VERTEX_BONE_WEIGHT_PACKED
float GetVertexBoneWeight(const in int2 boneWeights, int idx)
{
	const int bitShiftStep = 16;
	int weightIdx = 0;
	if (idx >= 2)
	{
		idx = idx - 2;
		weightIdx = 1;
	}

	const int bitShift = bitShiftStep * idx;
	const int boneWeightInt = boneWeights[weightIdx] >> bitShift;
	const float boneWeight = (float)(boneWeightInt & 0xFFFF) / 65535.0;
	return boneWeight;
}
#else
float GetVertexBoneWeight(const in float4 boneWeight, const in int idx)
{
	return boneWeight[idx];
}
#endif

struct GeoPixelInput
{
	float3 Position : SV_POSITION;
	float4 Normal : NORMAL0;
	float3 Colour : COLOR0;
	float2 UV : TEXCOORD0;
};