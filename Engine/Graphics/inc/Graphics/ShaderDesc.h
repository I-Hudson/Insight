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

			ShaderDesc(std::string shaderName, std::string shaderDataPath, ShaderStageFlags stages)
				: ShaderName(shaderName), ShaderDataPath(shaderDataPath), Stages(stages)
			{ }

			~ShaderDesc()
			{
			}

			std::string ShaderName = "";
			std::string ShaderDataPath;
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
#ifdef VERTEX_SPLIT_STREAMS
				return GetShaderInputLayoutFromStreams(Vertices::Stream::All);
#else
				return GetShaderInputLayoutFromStreams(Vertices::Stream::Interleaved);
#endif
			}

			static std::vector<ShaderInputLayout> GetShaderInputLayoutFromStreams(const Vertices::StreamFlags streams)
			{
				int vertexSlot = 0;

				std::vector<ShaderInputLayout> shaderInputLayout;
				
				if (streams & Vertices::Stream::Position
					|| streams & Vertices::Stream::All
					|| streams & Vertices::Stream::Interleaved)
				{
					shaderInputLayout.push_back(ShaderInputLayout(0, PixelFormat::R32G32B32_Float, vertexSlot, 0, "POSITION"));

#ifdef VERTEX_SPLIT_STREAMS
					++vertexSlot;
#endif
				}

				if (streams & Vertices::Stream::Normal
					|| streams & Vertices::Stream::All
					|| streams & Vertices::Stream::Interleaved)
				{
#ifdef VERTEX_NORMAL_PACKED
					shaderInputLayout.push_back(ShaderInputLayout(1, PixelFormat::R32_SInt, vertexSlot, 0, "NORMAL0"));
#else
					shaderInputLayout.push_back(ShaderInputLayout(1, PixelFormat::R32G32B32_Float, vertexSlot, 0, "NORMAL0"));
#endif

#ifdef VERTEX_SPLIT_STREAMS
					++vertexSlot;
#endif
				}

				if (streams & Vertices::Stream::Colour
					|| streams & Vertices::Stream::All
					|| streams & Vertices::Stream::Interleaved)
				{
#ifdef VERTEX_COLOUR_PACKED
					shaderInputLayout.push_back(ShaderInputLayout(2, PixelFormat::R32_SInt, vertexSlot, 0, "COLOR0"));
#else
					shaderInputLayout.push_back(ShaderInputLayout(2, PixelFormat::R32G32B32_Float, vertexSlot, 0, "COLOR0"));
#endif

#ifdef VERTEX_SPLIT_STREAMS
					++vertexSlot;
#endif
				}

				if (streams & Vertices::Stream::UV
					|| streams & Vertices::Stream::All
					|| streams & Vertices::Stream::Interleaved)
				{

#ifdef VERTEX_UV_PACKED
					shaderInputLayout.push_back(ShaderInputLayout(3, PixelFormat::R32_UInt, vertexSlot, 0, "TEXCOORD0"));
#else
					shaderInputLayout.push_back(ShaderInputLayout(3, PixelFormat::R32G32_Float, vertexSlot, 0, "TEXCOORD0"));
#endif

#ifdef VERTEX_SPLIT_STREAMS
					++vertexSlot;
#endif
				}

				if (streams & Vertices::Stream::BoneId
					|| streams & Vertices::Stream::All
					|| streams & Vertices::Stream::Interleaved)
				{
#ifdef VERTEX_BONE_ID_PACKED
					shaderInputLayout.push_back(ShaderInputLayout(4, PixelFormat::R32_SInt, vertexSlot, 0, "BLENDINDICES"));
#else
					shaderInputLayout.push_back(ShaderInputLayout(4, PixelFormat::R32G32B32A32_SInt, vertexSlot, 0, "BLENDINDICES"));
#endif

#ifdef VERTEX_SPLIT_STREAMS
					++vertexSlot;
#endif
				}

				if (streams & Vertices::Stream::BoneWeight
					|| streams & Vertices::Stream::All
					|| streams & Vertices::Stream::Interleaved)
				{
#ifdef VERTEX_BONE_WEIGHT_PACKED
					shaderInputLayout.push_back(ShaderInputLayout(5, PixelFormat::R32G32_SInt, vertexSlot, 0, "BLENDWEIGHT"));
#else
					shaderInputLayout.push_back(ShaderInputLayout(5, PixelFormat::R32G32B32A32_Float, vertexSlot, 0, "BLENDWEIGHT"));
#endif

#ifdef VERTEX_SPLIT_STREAMS
					++vertexSlot;
#endif
				}

				if (streams & Vertices::Stream::Interleaved)
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