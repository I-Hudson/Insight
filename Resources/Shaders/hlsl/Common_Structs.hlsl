#define MAX_BONE_COUNT 4

struct ShadowVertexInput
{
	float3 Position : POSITION;
	
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

struct GeoVertexInput
{
	float3 Position : POSITION;
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

#ifdef VERTEX_UV_PACKED
	int UV : TEXCOORD0;
#else
	float2 UV : TEXCOORD0;
#endif

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
float UnpackNormal(const in int normal, const in uint bitshift)
{
	const uint c_normalPackedValueBits = 0x1FF;
	const uint c_PackedNormalSignBit = 0x9;

	const uint normalData = normal >> bitshift;
	const uint signBit = (normalData >> c_PackedNormalSignBit) & 0x1;

	const uint normalPackedValue = normalData & c_normalPackedValueBits;

	const float sign = signBit == 0 ? 1.0 : -1.0;
	const float v = ((float)normalPackedValue / (float)c_normalPackedValueBits) * sign;
	return v;
}

float4 GetVertexNormal(const in int normal)
{
	const uint NormalPackedBitShiftInterval = 10;
	const uint NormalPackedMaxValue = 1U << NormalPackedBitShiftInterval;

	const float x = UnpackNormal(normal, 0);
	const float y = UnpackNormal(normal, NormalPackedBitShiftInterval);
	const float z = UnpackNormal(normal, NormalPackedBitShiftInterval * 2);

	return float4(x, y, z, 0.0);
}

#else
float4 GetVertexNormal(const in float3 normal)
{
	return float4(normal, 0.0);
}
#endif

#ifdef VERTEX_COLOUR_PACKED
float4 GetVertexColour(const in int colour)
{
	const float r = (float)(colour & 0xFF) / 255.0;
	const float g = (float)((colour >> 8) & 0xFF) / 255.0;
	const float b = (float)((colour >> 16) & 0xFF) / 255.0;
	return float4(r, g, b, 1.0);
}
#else
float4 GetVertexColour(const in float3 colour)
{
	return float4(colour, 1.0);
}
#endif

#ifdef VERTEX_UV_PACKED
float2 GetVertexUVs(const in int uv)
{
	const int xInt = uv & 0x0000FFFF;
	const int yInt = uv >> 16;

	const float xFloat = xInt / 65535.0;
	const float yFloat = yInt / 65535.0;
	return float2(xFloat, yFloat);
}
#else
float2 GetVertexUVs(const in float2 uv)
{
	return uv;
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