#pragma once

#include "Graphics/Defines.h"
#include "Graphics/PixelFormat.h"

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
					ShaderInputLayout(1, PixelFormat::R32G32B32_Float, 12, "NORMAL0"),
					ShaderInputLayout(2, PixelFormat::R32G32B32_Float, 24, "COLOR0"),
					ShaderInputLayout(3, PixelFormat::R32G32_Float, 36, "TEXCOORD0"),
				};
				return DefaultShaderInputLayout;
			}
		};
	}
}