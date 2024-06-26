#pragma once

#include "Graphics/Defines.h"
#include "Graphics/PixelFormat.h"
#include "Graphics/PixelFormatExtensions.h"

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
			ShaderInputLayout(int binding, PixelFormat format, int stride, std::string name)
				: Binding(binding), Format(format), Stride(stride), Name(std::move(name))
			{ }

			int Binding;
			PixelFormat Format;
			int Stride;
			std::string Name;

			u64 GetHash() const
			{
				u64 hash = 0;

				HashCombine(hash, Binding);
				HashCombine(hash, Format);
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
				std::vector<ShaderInputLayout> DefaultShaderInputLayout =
				{
					ShaderInputLayout(0, PixelFormat::R32G32B32_Float, 0, "POSITION"),
#ifdef VERTEX_NORMAL_PACKED
					ShaderInputLayout(1, PixelFormat::R32_SInt, 0, "NORMAL0"),
#else
					ShaderInputLayout(1, PixelFormat::R32G32B32_Float, 0, "NORMAL0"),
#endif

#ifdef VERTEX_COLOUR_PACKED
					ShaderInputLayout(2, PixelFormat::R32_SInt, 0, "COLOR0"),
#else
					ShaderInputLayout(2, PixelFormat::R32G32B32_Float, 0, "COLOR0"),
#endif

#ifdef VERTEX_UV_PACKED
					ShaderInputLayout(3, PixelFormat::R32_UInt, 0, "TEXCOORD0"),
#else
					ShaderInputLayout(3, PixelFormat::R32G32_Float, 0, "TEXCOORD0"),
#endif

#ifdef VERTEX_BONE_ID_PACKED
					ShaderInputLayout(4, PixelFormat::R32_SInt, 0, "BLENDINDICES"),
#else
					ShaderInputLayout(4, PixelFormat::R32G32B32A32_SInt, 0, "BLENDINDICES"),
#endif

#ifdef VERTEX_BONE_WEIGHT_PACKED
					ShaderInputLayout(5, PixelFormat::R32G32_SInt, 0, "BLENDWEIGHT"),
#else
					ShaderInputLayout(5, PixelFormat::R32G32B32A32_Float, 0, "BLENDWEIGHT"),
#endif
				};

				u64 offset = 0;
				for (ShaderInputLayout& inputLayout : DefaultShaderInputLayout)
				{
					inputLayout.Stride = offset;
					offset += PixelFormatExtensions::SizeInBytes(inputLayout.Format);
				}

				return DefaultShaderInputLayout;
			}
		};
	}
}