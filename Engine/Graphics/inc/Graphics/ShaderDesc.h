#pragma once

#include "Graphics/PixelFormat.h"

#include <string>
#include <vector>

namespace Insight
{
	namespace Graphics
	{
		struct ShaderInputLayout
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

		struct ShaderDesc
		{
			ShaderDesc()
			{ }

			ShaderDesc(std::string vertex, std::string pixel)
				: VertexFilePath(vertex), PixelFilePath(pixel)
			{ }

			~ShaderDesc()
			{
			}

			std::string VertexFilePath = "";
			std::string TesselationControlFilePath = "";
			std::string TesselationEvaluationVertexFilePath = "";
			std::string GeoemtyFilePath = "";
			std::string PixelFilePath = "";
			std::string MainFunc = "Main";
			std::vector<ShaderInputLayout> InputLayout; /// Optioanl, if left empty reflection will happen on the shader code.

			u64 GetHash() const
			{
				u64 hash = 0;
				
				if (!VertexFilePath.empty())
				{
					HashCombine(hash, VertexFilePath);
				}
				if (!TesselationControlFilePath.empty())
				{
					HashCombine(hash, TesselationControlFilePath);
				}
				if (!TesselationEvaluationVertexFilePath.empty())
				{
					HashCombine(hash, TesselationEvaluationVertexFilePath);
				} 
				if (!GeoemtyFilePath.empty())
				{
					HashCombine(hash, GeoemtyFilePath);
				}
				if (!PixelFilePath.empty())
				{
					HashCombine(hash, PixelFilePath);
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
				return !VertexFilePath.empty();
			}
		};
	}
}