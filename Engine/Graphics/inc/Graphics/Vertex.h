#pragma once

#include "Graphics/Defines.h"

#include "Core/TypeAlias.h"
#include "Maths/Vector2.h"
#include "Maths/Vector3.h"

#include "Core/Asserts.h"

#include <cstring>

namespace Insight
{
    namespace Graphics
    {
		static bool is_big_endian(void)
		{
			union {
				uint32_t i;
				char c[4];
			} bint = { 0x01020304 };

			return bint.c[0] == 1;
		}

		struct IS_GRAPHICS Vertex
		{
			const static uint32_t MAX_BONE_COUNT = 4;

			Vertex()
			{
				for (size_t boneIdx = 0; boneIdx < MAX_BONE_COUNT; ++boneIdx)
				{
#ifdef VERTEX_BONE_ID_PACKED
					BoneIds = 0;
#else
					BoneIds[boneIdx] = -1;
#endif
					BoneWeights[boneIdx] = 0.0f;
				}
			}
			Vertex(Maths::Vector3 pos, Maths::Vector3 nor, Maths::Vector3 colour, Maths::Vector2 uv)
			{
				Position[0] = pos[0];
				Position[1] = pos[1];
				Position[2] = pos[2];

#ifdef VERTEX_NORMAL_PACKED
				const u32 bitshiftInterval = 8;
				const u32 normalMaxValue = 1 << bitshiftInterval;

				PackNormal(nor.x, normalMaxValue, 0);
				PackNormal(nor.y, normalMaxValue, bitshiftInterval);
				PackNormal(nor.z, normalMaxValue, bitshiftInterval * 2);

				const float fX = UnpackNormal(Normal, normalMaxValue, 0);
				const float fY = UnpackNormal(Normal, normalMaxValue, bitshiftInterval);
				const float fZ = UnpackNormal(Normal, normalMaxValue, bitshiftInterval * 2);
#else
				Normal[0] = nor[0];
				Normal[1] = nor[1];
				Normal[2] = nor[2];
#endif

#ifdef VERTEX_COLOUR_PACKED
				Colour |= static_cast<u8>(colour.x * 255);
				Colour |= static_cast<u8>(colour.y * 255) << 8;
				Colour |= static_cast<u8>(colour.z * 255) << 16;
#else
				Colour[0] = colour[0];
				Colour[1] = colour[1];
				Colour[2] = colour[2];
#endif

#ifdef VERTEX_UV_PACKED
				const u16 xUVInt = PackNormalisedFloatToU16(uv.x, _UI16_MAX);
				const u16 yUVInt = PackNormalisedFloatToU16(uv.y, _UI16_MAX);
				
				UV |= xUVInt;
				UV |= yUVInt << 16;
#else
				UV[0] = uv[0];
				UV[1] = uv[1];
#endif

				for (size_t boneIdx = 0; boneIdx < MAX_BONE_COUNT; ++boneIdx)
				{
#ifdef VERTEX_BONE_ID_PACKED
					BoneIds = 0;
#else
					BoneIds[boneIdx] = 0;
#endif
					BoneWeights[boneIdx] = 0.0f;
				}
			}

			void SetBoneId(const u32 boneId, const u8 boneIdx)
			{
#ifdef VERTEX_BONE_ID_PACKED
				const u32 bitShiftStep = 8;
				const u32 bitShift = bitShiftStep * boneIdx;
				BoneIds |= boneId << bitShift;
#else
				BoneIds[boneIdx] = boneId;
#endif
			}

			float GetBoneWeight(const u8 idx) const
			{
#ifdef VERTEX_BONE_WEIGHT_PACKED
				const u32 bitShiftStep = 16;
				u16 packedWeight = 0;
				switch (idx)
				{
				case 0:
				{
					const u32 bitShift = bitShiftStep * 0;
					packedWeight |= (BoneWeights[0] >> bitShift) & 0xFFFF;
					break;
				}
				case 1:
				{
					const u32 bitShift = bitShiftStep * 1;
					packedWeight |= (BoneWeights[0] >> bitShift) & 0xFFFF;
					break;
				}
				case 2:
				{
					const u32 bitShift = bitShiftStep * 0;
					packedWeight |= (BoneWeights[1] >> bitShift) & 0xFFFF;
					break;
				}
				case 3:
				{
					const u32 bitShift = bitShiftStep * 1;
					packedWeight |= (BoneWeights[1] >> bitShift) & 0xFFFF;
					break;
				}
				default:
					break;
				}

				const float weight = (float)packedWeight / (float)_UI16_MAX;
				return weight;
#else
				return BoneWeights[idx];
#endif
			}
			void SetBoneWeight(const float boneWeight, const u8 idx)
			{
#ifdef VERTEX_BONE_WEIGHT_PACKED
				const int weight = static_cast<u16>(boneWeight * _UI16_MAX);
				const u32 bitShiftStep = 16;

				switch (idx)
				{
				case 0:
				{
					const u32 bitShift = bitShiftStep * 0;
					BoneWeights[0] |= weight << bitShift;
					break;
				}
				case 1:
				{
					const u32 bitShift = bitShiftStep * 1;
					BoneWeights[0] |= weight << bitShift;
					break;
				}
				case 2:
				{
					const u32 bitShift = bitShiftStep * 0;
					BoneWeights[1] |= weight << bitShift;
					break;
				}
				case 3:
				{
					const u32 bitShift = bitShiftStep * 1;
					BoneWeights[1] |= weight << bitShift;
					break;
				}
				default:
					break;
				}
#else
				BoneWeights[idx] = boneWeight;
#endif
			}

		private:
#ifdef VERTEX_NORMAL_PACKED
			const static u32 c_NormapPackBits = 0x3FE;//0b0000'0000'0000'0000'0000'0011'1111'1110;
				//const static u32 c_NormapPackBits = 0xFF;//0b0000'0000'0000'0000'0000'0000'1111'1110;
			
			void PackNormal(const float value, const u32 maxValue, const u8 bitshift)
				{
					ASSERT(std::abs(value) >= 0.0f && std::abs(value) <= 1.0f);
					ASSERT(bitshift < 21);
				
					if (value < 0.0f)
					{
						Normal |= 1 << bitshift;
					}
				
					const u32 normalData = static_cast<u32>(std::abs(value) * maxValue);
					Normal |= (normalData & c_NormapPackBits) << bitshift;
				}
			
			float UnpackNormal(const int normal, const u32 maxValue, const u8 bitshift) const
				{
					ASSERT(bitshift < 21);
					const u32 normalData = normal >> bitshift;
					const u32 normalValue = normalData & c_NormapPackBits;
				
					const int sign = (normalData & 0x1) == 0 ? 1 : -1;
				
					const float fValue = (static_cast<float>(normalValue) / static_cast<float>(maxValue)) * sign;
					return fValue;
				}
#endif
			u16 PackNormalisedFloatToU16(const float value, const u16 maxValue) const
			{
				ASSERT(std::abs(value) >= 0.0f && std::abs(value) <= 1.0f);
				const u16 intValue = value * maxValue;
				return intValue;
			}

			public:

			float Position[3] = { 0 };
#ifdef VERTEX_NORMAL_PACKED
			int Normal = 0;
#else
			float Normal[3] = { 0 };
#endif
#ifdef VERTEX_COLOUR_PACKED
			int Colour = 0;
#else
			float Colour[3] = { 0 };
#endif
#ifdef VERTEX_UV_PACKED
			int UV = 0;
#else
			float UV[2] = { 0 };
#endif
#ifdef VERTEX_BONE_ID_PACKED
			int BoneIds = 0;
#else
			int BoneIds[MAX_BONE_COUNT] = { 0 };
#endif
#ifdef VERTEX_BONE_WEIGHT_PACKED
			int BoneWeights[MAX_BONE_COUNT / 2] = { 0 };
#else
			float BoneWeights[MAX_BONE_COUNT] = { 0 };
#endif

			constexpr int GetStride() { return sizeof(Vertex); }
		};

		struct IS_GRAPHICS VertexBoneInfluence
		{
			VertexBoneInfluence()
			{
				for (size_t boneIdx = 0; boneIdx < Vertex::MAX_BONE_COUNT; ++boneIdx)
				{
#ifdef VERTEX_BONE_ID_PACKED
					BoneIds = 0;
#else
					BoneIds[boneIdx] = 0;
#endif
					BoneWeights[boneIdx] = 0.0f;
				}
			}

			void SetBoneId(const u32 boneId, const u8 boneIdx)
			{
#ifdef VERTEX_BONE_ID_PACKED
				const u32 bitShiftStep = 8;
				const u32 bitShift = bitShiftStep * boneIdx;
				BoneIds |= boneId << bitShift;
#else
				BoneIds[boneIdx] = boneId;
#endif
			}

			float GetBoneWeight(const u8 idx) const
			{
#ifdef VERTEX_BONE_WEIGHT_PACKED
				const u32 bitShiftStep = 16;
				u16 packedWeight = 0;
				switch (idx)
				{
				case 0:
				{
					const u32 bitShift = bitShiftStep * 0;
					packedWeight |= (BoneWeights[0] >> bitShift) & 0xFFFF;
					break;
				}
				case 1:
				{
					const u32 bitShift = bitShiftStep * 1;
					packedWeight |= (BoneWeights[0] >> bitShift) & 0xFFFF;
					break;
				}
				case 2:
				{
					const u32 bitShift = bitShiftStep * 0;
					packedWeight |= (BoneWeights[1] >> bitShift) & 0xFFFF;
					break;
				}
				case 3:
				{
					const u32 bitShift = bitShiftStep * 1;
					packedWeight |= (BoneWeights[1] >> bitShift) & 0xFFFF;
					break;
				}
				default:
					break;
				}

				const float weight = (float)packedWeight / (float)_UI16_MAX;
				return weight;
#else
				return BoneWeights[idx];
#endif
			}

			void SetBoneWeight(const float boneWeight, const u8 idx)
			{
#ifdef VERTEX_BONE_WEIGHT_PACKED
				const int weight = static_cast<u16>(boneWeight * _UI16_MAX);
				const u32 bitShiftStep = 16;

				switch (idx)
				{
				case 0:
				{
					const u32 bitShift = bitShiftStep * 0;
					BoneWeights[0] |= weight << bitShift;
					break;
				}
				case 1:
				{
					const u32 bitShift = bitShiftStep * 1;
					BoneWeights[0] |= weight << bitShift;
					break;
				}
				case 2:
				{
					const u32 bitShift = bitShiftStep * 0;
					BoneWeights[1] |= weight << bitShift;
					break;
				}
				case 3:
				{
					const u32 bitShift = bitShiftStep * 1;
					BoneWeights[1] |= weight << bitShift;
					break;
				}
				default:
					break;
				}
#else
				BoneWeights[idx] = boneWeight;
#endif
			}

#ifdef VERTEX_BONE_ID_PACKED
			int BoneIds = 0;
#else
			int BoneIds[MAX_BONE_COUNT] = { 0 };
#endif
#ifdef VERTEX_BONE_WEIGHT_PACKED
			int BoneWeights[MAX_BONE_COUNT / 2] = { 0 };
#else
			float BoneWeights[Vertex::MAX_BONE_COUNT] = { 0 };
#endif
		};

		/*
		struct IS_GRAPHICS VertexOptomised
		{
#define POS_BIT_MASK 0b1111'1111'1111'1111'0000'0000'0000'0000
#define POS_BIT_SHIFT 16

#define NOR_BIT_MASK 0b0000'0000'0011'1111'1111'1111'0000'0001
#define NOR_BIT_SHIFT 16

			VertexOptomised(Vertex v)
			{
				Position.x = GetShort(v.Position.x, POS_BIT_MASK, POS_BIT_SHIFT);
				Position.y = GetShort(v.Position.y, POS_BIT_MASK, POS_BIT_SHIFT);
				Position.z = GetShort(v.Position.z, POS_BIT_MASK, POS_BIT_SHIFT);

				short s = PackToShort(v.Position.x, POS_BIT_MASK, POS_BIT_SHIFT);

				float unpackedPositionX = GetFloat(Position.x, POS_BIT_SHIFT);
				float unpackedPositionY = GetFloat(Position.y, POS_BIT_SHIFT);
				float unpackedPositionZ = GetFloat(Position.z, POS_BIT_SHIFT);

				Normal.x = GetShort(v.Normal.x, NOR_BIT_MASK, NOR_BIT_SHIFT);
				//Normal.y = GetShortNormal(v.Normal.y, NOR_BIT_MASK, NOR_BIT_SHIFT);
				Normal.z = GetShort(v.Normal.z, NOR_BIT_MASK, NOR_BIT_SHIFT);
				
				//float unpackedNormalX = GetFloatNormal(Normal.x, NOR_BIT_MASK, NOR_BIT_SHIFT);
				//float unpackedNormalY = GetFloatNormal(Normal.y, NOR_BIT_MASK, NOR_BIT_SHIFT);
				float unpackedNormalz = GetFloat(Normal.z, NOR_BIT_SHIFT);
			}

		private:
			int16_t GetShort(float v, const int32_t bitMask, const uint8_t bitShift) const
			{
				int32_t position = 0;
				memcpy(&position, &v, 4);
				int32_t maskedPosition = position & bitMask;
				int16_t shortValue = maskedPosition >> bitShift;
				return shortValue;
			}

			int16_t PackToShort(float v, const int32_t bitMask, const uint8_t bitShift) const
			{
				int32_t valueAsIntger = 0;
				memcpy(&valueAsIntger, &v, 4);

				const uint32_t byteMasks[] =
				{
					0b0000'0000'0000'0000'0000'0000'1111'1111,
					0b0000'0000'0000'0000'1111'1111'0000'0000,
					0b0000'0000'1111'1111'0000'0000'0000'0000,
					0b1111'1111'0000'0000'0000'0000'0000'0000,
				};

				int16_t shortValue = 0;
				uint8_t bitShiftAmount = 0;
				uint8_t currentByteIdx = 0;
				for (size_t byteIdx = 0; byteIdx < 4; ++byteIdx)
				{
					// Get the mask for the byte at index byteIdx.
					const uint32_t mask = byteMasks[byteIdx];
					const uint32_t byteValue = valueAsIntger & mask;

					for (uint8_t bitIdx = 0; bitIdx < 8; ++bitIdx)
					{
						const int32_t currentBitIdx = (currentByteIdx * 8) + bitIdx;
						const uint32_t evaluateBitIdx = 1 << currentBitIdx;
						const bool validBit = bitMask & evaluateBitIdx;
						if (validBit)
						{
							assert(bitShiftAmount < 16);
							const int32_t bitValue = valueAsIntger & evaluateBitIdx;
							int32_t bitValueShifted = bitValue >> currentBitIdx;
							bitValueShifted = bitValueShifted << bitShiftAmount++;
							shortValue |= bitValueShifted;
						}
					}
					++currentByteIdx;
				}

				return shortValue;
			}

			int16_t GetShortNormal(float v, const int32_t bitMask, const uint8_t bitShift)
			{
				float rV = 0.5;
				int32_t rIntValue = 0;
				memcpy(&rIntValue, &rV, 4);

				bool sign = std::signbit(v);
				int32_t intValue = 0;
				memcpy(&intValue, &v, 4);


				int32_t maskedPosition = intValue & bitMask;
				int16_t shortValue = maskedPosition >> bitShift;
				shortValue |= (sign << 15);

				return shortValue;
			}

			float GetFloat(int16_t v, const uint8_t bitShift)
			{
				int32_t packedPosition = 0;
				memcpy(&packedPosition, &v, 2);
				packedPosition = packedPosition << bitShift;

				float position = 0;
				memcpy(&position, &packedPosition, 4);
				return position;
			}

			float GetFloatNormal(int16_t v, const uint8_t bitShift)
			{
				int32_t sign = (v & 0x8000) >> 15;
				v &= ~(1 << 15);
				if (sign == 1)
				{
					v = ~v;
				}

				int32_t packedValue = 0;
				memcpy(&packedValue, &v, 2);
				packedValue = packedValue << bitShift;

				float floatValue = 0;
				memcpy(&floatValue, &packedValue, 4);
				return floatValue;
			}

			Maths::Vector3 Position;
			Maths::Vector3 Normal;
			Maths::Vector3 Colour;
			Maths::Vector2 UV;
		};
		*/
    }
}