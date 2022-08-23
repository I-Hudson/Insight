#pragma once

#include "Graphics/PixelFormat.h"

#include "Core/Profiler.h"

#include <string>

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
				return hash;
			}

			bool IsValid() const
			{
				return !VertexFilePath.empty();
			}
		};
	}
}