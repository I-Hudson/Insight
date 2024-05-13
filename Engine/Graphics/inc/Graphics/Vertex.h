#pragma once

#include "Graphics/Defines.h"

#include "Maths/Vector2.h"
#include "Maths/Vector3.h"

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
			Vertex()
			{ }
			Vertex(Maths::Vector3 pos, Maths::Vector3 nor, Maths::Vector3 colour, Maths::Vector2 uv)
			{
				Position[0] = pos[0];
				Position[1] = pos[1];
				Position[2] = pos[2];

				Normal[0] = nor[0];
				Normal[1] = nor[1];
				Normal[2] = nor[2];

				Colour[0] = colour[0];
				Colour[1] = colour[1];
				Colour[2] = colour[2];

				UV[0] = uv[0];
				UV[1] = uv[1];
			}

			float Position[3];
			float Normal[3];
			float Colour[3];
			float UV[2];

			constexpr int GetStride() { return sizeof(Vertex); }
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