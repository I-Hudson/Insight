#pragma once

#include <string>
#include "Graphics/PixelFormat.h"

namespace Insight
{
	namespace Graphics
	{
		struct ShaderInputLayout
		{
			ShaderInputLayout()
			{ }
			ShaderInputLayout(int binding, PixelFormat format, int stride, std::string name = "")
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
				HashCombine(hash, Name);

				return hash;
			}
		};
		struct ShaderDesc
		{
			ShaderDesc()
			{ }

			ShaderDesc(std::wstring vertex, std::wstring pixel)
				: VertexFilePath(vertex), PixelFilePath(pixel)
			{ }

			~ShaderDesc()
			{
			}

			std::wstring VertexFilePath = L"";
			std::wstring TesselationControlFilePath = L"";
			std::wstring TesselationEvaluationVertexFilePath = L"";
			std::wstring GeoemtyFilePath = L"";
			std::wstring PixelFilePath = L"";
			std::string MainFunc = "Main";

			u64 GetHash() const
			{
				u64 hash = 0;

				HashCombine(hash, VertexFilePath);
				HashCombine(hash, TesselationControlFilePath);
				HashCombine(hash, TesselationEvaluationVertexFilePath);
				HashCombine(hash, GeoemtyFilePath);
				HashCombine(hash, PixelFilePath);
				HashCombine(hash, MainFunc);
				return hash;
			}

			bool IsValid() const
			{
				return !VertexFilePath.empty();
			}
		};
	}
}