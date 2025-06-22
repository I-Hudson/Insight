#pragma once

#include "Graphics/Defines.h"
#include "Graphics/PixelFormat.h"
#include "Graphics/PixelFormatExtensions.h"

#include "Graphics/Vertex.h"

#include <string>
#include <vector>

namespace Insight
{
	namespace Graphics
	{
		struct IS_GRAPHICS ShaderInputLayout
		{
			ShaderInputLayout()
			{ }
			ShaderInputLayout(const int binding, const PixelFormat format, const int slot, int stride, const std::string name)
				: Binding(binding), Format(format), Slot(slot), Stride(stride), Name(std::move(name))
			{ }

			int Binding;
			PixelFormat Format;
			int Slot;
			int Stride;
			std::string Name;

			u64 GetHash() const
			{
				u64 hash = 0;

				HashCombine(hash, Binding);
				HashCombine(hash, Format);
				HashCombine(hash, Slot);
				HashCombine(hash, Stride);

				return hash;
			}
		};

		struct IS_GRAPHICS ShaderDesc
		{
			ShaderDesc()
			{ }

			ShaderDesc(std::string shaderName, std::vector<Byte> shaderData, ShaderStageFlags stages)
				: ShaderName(shaderName), ShaderData(shaderData), Stages(stages)
			{ }

			~ShaderDesc()
			{
			}

			std::string ShaderName = "";
			std::vector<Byte> ShaderData;
			ShaderStageFlags Stages;

			std::string MainFunc = "Main";
			std::vector<ShaderInputLayout> InputLayout; /// Optioanl, if left empty reflection will happen on the shader code.

			u64 GetHash() const
			{
				u64 hash = 0;
				
				if (!ShaderName.empty())
				{
					HashCombine(hash, ShaderName);
				}
				if (!MainFunc.empty())
				{
					HashCombine(hash, MainFunc);
				}
				
				for (const ShaderInputLayout& layout : InputLayout)
				{
					HashCombine(hash, layout.GetHash());
				}

				return hash;
			}

			bool IsValid() const
			{
				return !ShaderName.empty();
			}

			static std::vector<ShaderInputLayout> GetDefaultShaderInputLayout()
			{
#ifdef VERTEX_SPLIT_STREAM
				return GetShaderInputLayoutFromStreams(Vertices::Stream::All);
#else
				return GetShaderInputLayoutFromStreams(Vertices::Stream::Interleaved);
#endif
			}

			static std::vector<ShaderInputLayout> GetShaderInputLayoutFromStreams(const Vertices::Stream streams)
			{
#ifdef VERTEX_SPLIT_STREAM
				const int positionSlot = 0;
				const int normalSlot = 1;
				const int colourSlot = 2;
				const int uvSlot = 3;
				const int boneIdsSlot = 4;
				const int boneWeightsSlot = 5;
#else
				const int positionSlot = 0;
				const int normalSlot = 0;
				const int colourSlot = 0;
				const int uvSlot = 0;
				const int boneIdsSlot = 0;
				const int boneWeightsSlot = 0;

#endif
				std::vector<ShaderInputLayout> shaderInputLayout;
				
				if ((streams & Vertices::Stream::Position) == 0
					|| (streams & Vertices::Stream::All) == 0
					|| (streams & Vertices::Stream::Interleaved) == 0)
				{
					shaderInputLayout.push_back(ShaderInputLayout(0, PixelFormat::R32G32B32_Float, positionSlot, 0, "POSITION"));
				}

				if ((streams & Vertices::Stream::Normal) == 0
					|| (streams & Vertices::Stream::All) == 0
					|| (streams & Vertices::Stream::Interleaved) == 0)
				{
#ifdef VERTEX_NORMAL_PACKED
					shaderInputLayout.push_back(ShaderInputLayout(1, PixelFormat::R32_SInt, normalSlot, 0, "NORMAL0"));
#else
					shaderInputLayout.push_back(ShaderInputLayout(1, PixelFormat::R32G32B32_Float, normalSlot, 0, "NORMAL0"));
#endif
				}

				if ((streams & Vertices::Stream::Colour) == 0
					|| (streams & Vertices::Stream::All) == 0
					|| (streams & Vertices::Stream::Interleaved) == 0)
				{
#ifdef VERTEX_COLOUR_PACKED
					shaderInputLayout.push_back(ShaderInputLayout(2, PixelFormat::R32_SInt, colourSlot, 0, "COLOR0"));
#else
					shaderInputLayout.push_back(ShaderInputLayout(2, PixelFormat::R32G32B32_Float, colourSlot, 0, "COLOR0"));
#endif
				}

				if ((streams & Vertices::Stream::UV) == 0
					|| (streams & Vertices::Stream::All) == 0
					|| (streams & Vertices::Stream::Interleaved) == 0)
				{

#ifdef VERTEX_UV_PACKED
					shaderInputLayout.push_back(ShaderInputLayout(3, PixelFormat::R32_UInt, uvSlot, 0, "TEXCOORD0"));
#else
					shaderInputLayout.push_back(ShaderInputLayout(3, PixelFormat::R32G32_Float, uvSlot, 0, "TEXCOORD0"));
#endif
				}

				if ((streams & Vertices::Stream::BoneId) == 0
					|| (streams & Vertices::Stream::All) == 0
					|| (streams & Vertices::Stream::Interleaved) == 0)
				{
#ifdef VERTEX_BONE_ID_PACKED
					shaderInputLayout.push_back(ShaderInputLayout(4, PixelFormat::R32_SInt, boneIdsSlot, 0, "BLENDINDICES"));
#else
					shaderInputLayout.push_back(ShaderInputLayout(4, PixelFormat::R32G32B32A32_SInt, boneIdsSlot, 0, "BLENDINDICES"));
#endif
				}

				if ((streams & Vertices::Stream::BoneWeight) == 0
					|| (streams & Vertices::Stream::All) == 0
					|| (streams & Vertices::Stream::Interleaved) == 0)
				{
#ifdef VERTEX_BONE_WEIGHT_PACKED
					shaderInputLayout.push_back(ShaderInputLayout(5, PixelFormat::R32G32_SInt, boneWeightsSlot, 0, "BLENDWEIGHT"));
#else
					shaderInputLayout.push_back(ShaderInputLayout(5, PixelFormat::R32G32B32A32_Float, boneWeightsSlot, 0, "BLENDWEIGHT"));
#endif
				}

				if ((streams & Vertices::Stream::Interleaved) == 0)
				{
					int offset = 0;
					for (ShaderInputLayout& inputLayout : shaderInputLayout)
					{
						inputLayout.Stride = offset;
						offset += PixelFormatExtensions::SizeInBytes(inputLayout.Format);
					}
				}

				return shaderInputLayout;
			}
		};
	}
}