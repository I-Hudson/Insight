#pragma once

#include "Graphics/Defines.h"

#include "Core/TypeAlias.h"
#include "Maths/Vector2.h"
#include "Maths/Vector3.h"

#include "Core/Asserts.h"

#include "Graphics/RHI/RHI_Buffer.h"

#include <cstring>
#include <vector>

namespace Insight
{
	namespace Graphics
	{
		class RHI_Buffer;

		static bool is_big_endian(void)
		{
			union {
				uint32_t i;
				char c[4];
			} bint = { 0x01020304 };

			return bint.c[0] == 1;
		}

		struct IS_GRAPHICS Vertices
		{
			const static uint32_t MAX_BONE_COUNT = 4;

			enum Stream
			{
				Position		= BIT(0),
				Normal			= BIT(1),
				Colour			= BIT(2),
				UV				= BIT(3),
				BoneId			= BIT(4),
				BoneWeight		= BIT(5),

				All				= BIT(6),
				Interleaved		= BIT(7),
			};
			using StreamFlags = u64;
			constexpr static const char* StreamNames[] =
			{
				"Position",
				"Normal",
				"Colour",
				"UV",
				"BoneId",
				"BoneWeight",

				"All",
				"Interleaved",
			};
			constexpr static int StreamCount = ARRAY_COUNT(StreamNames);

			struct Vertex
			{
#ifdef VERTEX_SPLIT_STREAMS
				std::vector<float> Positons;
#else
				float Position[3] = { 0 };
#endif

#ifdef VERTEX_SPLIT_STREAMS
#ifdef VERTEX_NORMAL_PACKED
				std::vector<int> Normals;
#else
				std::vector<float> Normals;
#endif
#else
#ifdef VERTEX_NORMAL_PACKED
				int Normal = 0;
#else
				float Normal[3] = { 0 };
#endif
#endif

#ifdef VERTEX_SPLIT_STREAMS
#ifdef VERTEX_COLOUR_PACKED
				std::vector<int> Colours;
#else
				std::vector<float> Colours;
#endif
#else
#ifdef VERTEX_COLOUR_PACKED
				int Colour = 0;
#else
				float Colour[3] = { 0 };
#endif
#endif

#ifdef VERTEX_SPLIT_STREAMS
#ifdef VERTEX_UV_PACKED
				std::vector<int> UVs;
#else
				std::vector<float> UVs;
#endif
#else
#ifdef VERTEX_UV_PACKED
				int UV = 0;
#else
				float UV[2] = { 0 };
#endif
#endif

#ifdef VERTEX_SPLIT_STREAMS
#ifdef VERTEX_BONE_ID_PACKED
				std::vector<int> BoneIds;
#else
				std::vector<float> BoneIds;
#endif
#else
#ifdef VERTEX_BONE_ID_PACKED
				int BoneIds = 0;
#else
				int BoneIds[MAX_BONE_COUNT] = { 0 };
#endif
#endif

#ifdef VERTEX_SPLIT_STREAMS
#ifdef VERTEX_BONE_WEIGHT_PACKED
				std::vector<int> BoneWeights;
#else
				std::vector<float> BoneWeights;
#endif
#else
#ifdef VERTEX_BONE_WEIGHT_PACKED
				int BoneWeights[MAX_BONE_COUNT / 2] = { 0 };
#else
				float BoneWeights[MAX_BONE_COUNT] = { 0 };
#endif
#endif
			};

			void AddVertex(const Maths::Vector3& position, const Maths::Vector3& normal, const Maths::Vector4& colour, const Maths::Vector2& uv, const int boneId, const Maths::Vector4& boneWeights)
			{
#ifdef VERTEX_SPLIT_STREAMS
				Vertices.Positons.push_back(position[0]);
				Vertices.Positons.push_back(position[1]);
				Vertices.Positons.push_back(position[2]);

#ifdef VERTEX_NORMAL_PACKED
				const u32 bitShift = c_PackedNormalSignBit + 1;

				int& packedNormal = Vertices.Normals.emplace_back();
				PackNormal(packedNormal, normal.x, 0);
				PackNormal(packedNormal, normal.y, bitShift);
				PackNormal(packedNormal, normal.z, bitShift * 2);

				const float fX = UnpackNormal(packedNormal, 0);
				const float fY = UnpackNormal(packedNormal, bitShift);
				const float fZ = UnpackNormal(packedNormal, bitShift * 2);

				ASSERT(Maths::Equals(fX, normal.x, 0.01f));
				ASSERT(Maths::Equals(fY, normal.y, 0.01f));
				ASSERT(Maths::Equals(fZ, normal.z, 0.01f));
#else
				Vertices.Normals.emplace_back(normal[0]);
				Vertices.Normals.emplace_back(normal[1]);
				Vertices.Normals.emplace_back(normal[2]);
#endif

#ifdef VERTEX_COLOUR_PACKED
				int& packedColour = Vertices.Colours.emplace_back();
				packedColour |= static_cast<u8>(colour.x * 255);
				packedColour |= static_cast<u8>(colour.y * 255) << 8;
				packedColour |= static_cast<u8>(colour.z * 255) << 16;
#else
				Vertices.Colours.emplace_back(colour[0]);
				Vertices.Colours.emplace_back(colour[1]);
				Vertices.Colours.emplace_back(colour[2]);
#endif

#ifdef VERTEX_UV_PACKED
				const u16 xUVInt = PackNormalisedFloatToU16(uv.x, _UI16_MAX);
				const u16 yUVInt = PackNormalisedFloatToU16(uv.y, _UI16_MAX);

				int& packedUV = Vertices.UVs.emplace_back();
				packedUV |= xUVInt;
				packedUV |= yUVInt << 16;
#else
				Vertices.UVs.emplace_back(uv[0]);
				Vertices.UVs.emplace_back(uv[1]);
#endif

#ifdef VERTEX_BONE_ID_PACKED
				Vertices.BoneIds.emplace_back() = 0;
#else
				for (size_t i = 0; i < MAX_BONE_COUNT; i++)
				{
					Vertices.BoneIds.emplace_back() = 0;
				}
#endif
				for (size_t i = 0; i < MAX_BONE_COUNT; i++)
				{
					Vertices.BoneWeights.emplace_back() = 0.0f;
				}

#else
				Vertex& vertex = Vertices.emplace_back();
				vertex.Position[0] = position[0];
				vertex.Position[1] = position[1];
				vertex.Position[2] = position[2];

#ifdef VERTEX_NORMAL_PACKED
				const u32 bitShift = c_PackedNormalSignBit + 1;

				PackNormal(vertex.Normal, normal.x, 0);
				PackNormal(vertex.Normal, normal.y, bitShift);
				PackNormal(vertex.Normal, normal.z, bitShift * 2);

				const float fX = UnpackNormal(vertex.Normal, 0);
				const float fY = UnpackNormal(vertex.Normal, bitShift);
				const float fZ = UnpackNormal(vertex.Normal, bitShift * 2);

				ASSERT(Maths::Equals(fX, normal.x, 0.01f));
				ASSERT(Maths::Equals(fY, normal.y, 0.01f));
				ASSERT(Maths::Equals(fZ, normal.z, 0.01f));
#else
				Normal[0] = normal[0];
				Normal[1] = normal[1];
				Normal[2] = normal[2];
#endif

#ifdef VERTEX_COLOUR_PACKED
				vertex.Colour |= static_cast<u8>(colour.x * 255);
				vertex.Colour |= static_cast<u8>(colour.y * 255) << 8;
				vertex.Colour |= static_cast<u8>(colour.z * 255) << 16;
#else
				vertex.Colour[0] = colour[0];
				vertex.Colour[1] = colour[1];
				vertex.Colour[2] = colour[2];
#endif

#ifdef VERTEX_UV_PACKED
				const u16 xUVInt = PackNormalisedFloatToU16(uv.x, _UI16_MAX);
				const u16 yUVInt = PackNormalisedFloatToU16(uv.y, _UI16_MAX);

				UV |= xUVInt;
				UV |= yUVInt << 16;
#else
				vertex.UV[0] = uv[0];
				vertex.UV[1] = uv[1];
#endif

				for (size_t boneIdx = 0; boneIdx < MAX_BONE_COUNT; ++boneIdx)
				{
#ifdef VERTEX_BONE_ID_PACKED
					vertex.BoneIds = 0;
#else
					BoneIds[boneIdx] = 0;
#endif
					vertex.BoneWeights[boneIdx] = 0.0f;
				}
#endif
			}

			void SetBoneId(const u64 vertexId, const u32 boneId, const u8 boneIdx)
			{
#ifdef VERTEX_SPLIT_STREAMS
#ifdef VERTEX_BONE_ID_PACKED
				int& vBoneId = Vertices.BoneIds[vertexId];
#else
				float& vBoneId = Vertices.BoneIds[vertexId];
#endif

#else
#ifdef VERTEX_BONE_ID_PACKED
				int& vBoneId = vertex.BoneIds[boneIdx];
#else
				float& vBoneId = vertex.BoneIds[boneIdx];
#endif

#endif

#ifdef VERTEX_BONE_ID_PACKED
				const u32 bitShiftStep = 8;
				const u32 bitShift = bitShiftStep * boneIdx;
				vBoneId |= boneId << bitShift;
#else
				vBoneId = boneId;
#endif
			}

			float GetBoneWeight(const u64 vertexId, const u8 idx) const
			{
#ifdef VERTEX_SPLIT_STREAMS
				const u64 boneWeightStartIdx = vertexId * MAX_BONE_COUNT;

#ifdef VERTEX_BONE_WEIGHT_PACKED
				int* boneWeights = nullptr;
				FAIL_ASSERT();
#else
				const float* boneWeights = Vertices.BoneWeights.data() + boneWeightStartIdx;
#endif
#else
				const Vertex& vertex = Vertices[vertexId];
#ifdef VERTEX_BONE_WEIGHT_PACKED
				int* boneWeights = nullptr;
				FAIL_ASSERT();
#else
				const float* boneWeights = vertex.BoneWeights;
#endif

#endif

#ifdef VERTEX_BONE_WEIGHT_PACKED
				const u32 bitShiftStep = 16;
				u16 packedWeight = 0;
				switch (idx)
				{
				case 0:
				{
					const u32 bitShift = bitShiftStep * 0;
					packedWeight |= (boneWeights[0] >> bitShift) & 0xFFFF;
					break;
				}
				case 1:
				{
					const u32 bitShift = bitShiftStep * 1;
					packedWeight |= (boneWeights[0] >> bitShift) & 0xFFFF;
					break;
				}
				case 2:
				{
					const u32 bitShift = bitShiftStep * 0;
					packedWeight |= (boneWeights[1] >> bitShift) & 0xFFFF;
					break;
				}
				case 3:
				{
					const u32 bitShift = bitShiftStep * 1;
					packedWeight |= (boneWeights[1] >> bitShift) & 0xFFFF;
					break;
				}
				default:
					break;
				}

				const float weight = (float)packedWeight / (float)_UI16_MAX;
				return weight;
#else
				return boneWeights[idx];
#endif
			}

			void SetBoneWeight(const u64 vertexId, const float boneWeight, const u8 idx)
			{
#ifdef VERTEX_SPLIT_STREAMS
				const u64 boneWeightStartIdx = vertexId * MAX_BONE_COUNT;

#ifdef VERTEX_BONE_WEIGHT_PACKED
				int* boneWeights = nullptr;
				FAIL_ASSERT();
#else
				float* boneWeights = Vertices.BoneWeights.data() + boneWeightStartIdx;
#endif
#else
				Vertex& vertex = Vertices[vertexId];
#ifdef VERTEX_BONE_WEIGHT_PACKED
				int* boneWeights = nullptr;
				FAIL_ASSERT();
#else
				float* boneWeights = vertex.BoneWeights;
#endif

#endif

#ifdef VERTEX_BONE_WEIGHT_PACKED
				const int weight = static_cast<u16>(boneWeight * _UI16_MAX);
				const u32 bitShiftStep = 16;

				switch (idx)
				{
				case 0:
				{
					const u32 bitShift = bitShiftStep * 0;
					boneWeights[0] |= weight << bitShift;
					break;
				}
				case 1:
				{
					const u32 bitShift = bitShiftStep * 1;
					boneWeights[0] |= weight << bitShift;
					break;
				}
				case 2:
				{
					const u32 bitShift = bitShiftStep * 0;
					boneWeights[1] |= weight << bitShift;
					break;
				}
				case 3:
				{
					const u32 bitShift = bitShiftStep * 1;
					boneWeights[1] |= weight << bitShift;
					break;
				}
				default:
					break;
				}
#else
				boneWeights[idx] = boneWeight;
#endif
			}

			void* GetData(const Stream stream)
			{
				switch (stream)
				{
				case Stream::Position:
				{
#ifdef VERTEX_SPLIT_STREAMS
					return Vertices.Positons.data();
#else
					return Vertices[0].Position;
#endif
				}
				case Stream::Normal:
				{
#ifdef VERTEX_SPLIT_STREAMS
					return Vertices.Normals.data();

#else
#ifdef VERTEX_NORMAL_PACKED
					return &Vertices[0].Normal;
#else
					return Vertices[0].Normal;
#endif
#endif
				}
				case Stream::Colour:
				{
#ifdef VERTEX_SPLIT_STREAMS
					return Vertices.Colours.data();

#else
#ifdef VERTEX_COLOUR_PACKED
					return &Vertices[0].Colour;
#else
					return Vertices[0].Colour;
#endif
#endif
				}
				case Stream::UV:
				{
#ifdef VERTEX_SPLIT_STREAMS
					return Vertices.UVs.data();

#else
#ifdef VERTEX_UV_PACKED
					return &Vertices[0].UV;
#else
					return Vertices[0].UV;
#endif
#endif
				}
				case Stream::BoneId:
				{
#ifdef VERTEX_SPLIT_STREAMS
					return Vertices.BoneIds.data();

#else
#ifdef VERTEX_BONE_ID_PACKED
					return &Vertices[0].BoneIds;
#else
					return Vertices[0].BoneIds;
#endif
#endif
				}
				case Stream::BoneWeight:
				{
#ifdef VERTEX_SPLIT_STREAMS
					return Vertices.BoneWeights.data();
#else
					return Vertices[0].BoneWeights;
#endif
				}

				case Stream::Interleaved:
				{
#ifdef VERTEX_SPLIT_STREAMS
					return Vertices.Positons.data();
#else
					return Vertices[0].Position;
#endif
				}

				default:
					return nullptr;
				}
			}

			const void* GetData(const Stream stream) const
			{
				return const_cast<Graphics::Vertices*>(this)->GetData(stream);
			}

			static int GetStride(const Stream stream)
			{
				int size = 0;
				switch (stream)
				{
				case Stream::Position:
				{
#ifdef VERTEX_SPLIT_STREAMS
					return sizeof(decltype(Vertex::Positons)::value_type) * 3;
#else
					return sizeof(Vertex::Position);
#endif
				}
				case Stream::Normal:
				{
#ifdef VERTEX_SPLIT_STREAMS
#ifdef VERTEX_NORMAL_PACKED
					return sizeof(decltype(Vertex::Normals)::value_type);
#else
					return sizeof(decltype(Vertex::Normals)::value_type) * 3;
#endif
#else
					return sizeof(Vertex::Normal);
#endif
				}
				case Stream::Colour:
				{
#ifdef VERTEX_SPLIT_STREAMS
#ifdef VERTEX_COLOUR_PACKED
					return sizeof(decltype(Vertex::Colours)::value_type);
#else
					return sizeof(decltype(Vertex::Colours)::value_type) * 3;
#endif
#else
					return sizeof(Vertex::Colour);
#endif
				}
				case Stream::UV:
				{
#ifdef VERTEX_SPLIT_STREAMS
#ifdef VERTEX_UV_PACKED
					return sizeof(decltype(Vertex::UVs)::value_type);
#else
					return sizeof(decltype(Vertex::UVs)::value_type) * 2;
#endif
#else
					return sizeof(Vertex::UV);
#endif
				}
				case Stream::BoneId:
				{
#ifdef VERTEX_SPLIT_STREAMS
#ifdef VERTEX_BONE_ID_PACKED
					return sizeof(decltype(Vertex::BoneIds)::value_type);
#else
					return sizeof(decltype(Vertex::BoneIds)::value_type) * MAX_BONE_COUNT;
#endif
#else
					return sizeof(Vertex::BoneIds);
#endif
				}
				case Stream::BoneWeight:
				{
#ifdef VERTEX_SPLIT_STREAMS
#ifdef VERTEX_BONE_WEIGHT_PACKED
					return sizeof(decltype(Vertex::BoneWeights)::value_type) * (MAX_BONE_COUNT / 2);
#else
					return sizeof(decltype(Vertex::BoneWeights)::value_type) * MAX_BONE_COUNT;
#endif
#else
					return sizeof(Vertex::BoneWeights);
#endif
				}
				case Stream::Interleaved:
				{
#ifdef VERTEX_SPLIT_STREAMS
					FAIL_ASSERT();
#else
					return sizeof(Vertex);
#endif
				}

				default:
					return 0;
				}
			}

			u64 VerticesCount() const 
			{ 
#ifdef VERTEX_SPLIT_STREAMS
				return Vertices.Positons.size() / 3;
#else
				return Vertices.size();
#endif
			}

			bool IsEmpty() const
			{
#ifdef VERTEX_SPLIT_STREAMS
				return Vertices.Positons.empty();
#else
				return Vertices.empty();
#endif
			}

			void Resize(const int size)
			{
#ifdef VERTEX_SPLIT_STREAMS
				Vertices.Positons.resize(size * 3);

#ifdef VERTEX_NORMAL_PACKED
				Vertices.Normals.resize(size);
#else
				Vertices.Normals.resize(size * 3);
#endif

#ifdef VERTEX_COLOUR_PACKED
				Vertices.Colours.resize(size);
#else
				Vertices.Colours.resize(size * 3);
#endif

#ifdef VERTEX_UV_PACKED
				Vertices.UVs.resize(size);
#else
				Vertices.UVs.resize(size * 2);
#endif

#ifdef VERTEX_BONE_ID_PACKED
				Vertices.BoneIds.resize(size);
#else
				Vertices.Normals.resize(size * 4);
#endif

#ifdef VERTEX_BONE_WEIGHT_PACKED
				Vertices.BoneWeights.resize(size);
#else
				Vertices.BoneWeights.resize(size * 4);
#endif

#else
				Vertices.resize(size);
#endif
			}

			private:
#ifdef VERTEX_NORMAL_PACKED
				const static u32 c_NormapPackBits = 0x1FF;		//0b0000'0000'0000'0000'0000'0001'1111'1111;
				const static u32 c_PackedNormalSignBit = 0x9;	//0b0000'0000'0000'0000'0000'0010'0000'0000;

				void PackNormal(int& packedNormal, const float value, const u8 bitshift)
				{
					ASSERT(std::abs(value) >= 0.0f && std::abs(value) <= 1.0f);
					ASSERT(bitshift <= 20);
					u32 normalData = 0;

					if (value < 0.0f)
					{
						normalData |= 1 << c_PackedNormalSignBit;
					}

					const u32 normalValue = static_cast<u32>(std::abs(value) * c_NormapPackBits);
					normalData |= normalValue & c_NormapPackBits;
					packedNormal |= normalData << bitshift;
				}

				float UnpackNormal(const int normal, const u8 bitshift) const
				{
					ASSERT(bitshift <= 20);
					const u32 normalData = normal >> bitshift;

					const u32 signBit = (normalData >> c_PackedNormalSignBit) & 0x1;
					const u32 normalPackedValue = normalData & c_NormapPackBits;

					const int sign = signBit == 0 ? 1 : -1;

					const float fValue = (static_cast<float>(normalPackedValue) / static_cast<float>(c_NormapPackBits)) * sign;
					return fValue;
				}
#endif
				u16 PackNormalisedFloatToU16(const float value, const u16 maxValue) const
				{
					ASSERT(std::abs(value) >= 0.0f && std::abs(value) <= 1.0f);
					const u16 intValue = static_cast<u16>(value * maxValue);
					return intValue;
				}

#ifdef VERTEX_SPLIT_STREAMS
			Vertex Vertices;
#else
			std::vector<Vertex> Vertices;
#endif
		};

		struct IS_GRAPHICS VertexBoneInfluence
		{
			VertexBoneInfluence()
			{
				for (size_t boneIdx = 0; boneIdx < Vertices::MAX_BONE_COUNT; ++boneIdx)
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
			int BoneIds[Vertex::MAX_BONE_COUNT] = { 0 };
#endif
#ifdef VERTEX_BONE_WEIGHT_PACKED
			int BoneWeights[MAX_BONE_COUNT / 2] = { 0 };
#else
			float BoneWeights[Vertices::MAX_BONE_COUNT] = { 0 };
#endif
		};

		struct IS_GRAPHICS VerticesSplitRHIBuffers
		{
			RHI_BufferView PositionView;
			RHI_BufferView NormalView;
			RHI_BufferView ColourView;
			RHI_BufferView UVView;
			RHI_BufferView BoneIdsView;
			RHI_BufferView BoneWeightsView;

			VerticesSplitRHIBuffers() = default;
			VerticesSplitRHIBuffers(RHI_Buffer* position, RHI_Buffer* normal, RHI_Buffer* colour, RHI_Buffer* uv, RHI_Buffer* boneIds, RHI_Buffer* boneWeights)
				: Position(position), Normal(normal), Colour(colour), UV(uv), BoneIds(boneIds), BoneWeights(boneWeights)
			{
			}

			RHI_Buffer* GetBuffer(const Vertices::Stream stream) const
			{
				switch (stream)
				{
				case Vertices::Stream::Position: return Position;
				case Vertices::Stream::Normal: return Normal;
				case Vertices::Stream::Colour: return Colour;
				case Vertices::Stream::UV: return UV;
				case Vertices::Stream::BoneId: return BoneIds;
				case Vertices::Stream::BoneWeight: return BoneWeights;
				default:
					FAIL_ASSERT();
					return nullptr;
				}
			}

			Graphics::RHI_Buffer* Position = nullptr;
			Graphics::RHI_Buffer* Normal = nullptr;
			Graphics::RHI_Buffer* Colour = nullptr;
			Graphics::RHI_Buffer* UV = nullptr;
			Graphics::RHI_Buffer* BoneIds = nullptr;
			Graphics::RHI_Buffer* BoneWeights = nullptr;
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