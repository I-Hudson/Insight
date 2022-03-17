#pragma once

#include "Core/TypeAlias.h"
#include "Graphics/PixelFormat.h"

#include <string>
#include <map>
#include <vector>

namespace Insight
{
	namespace Graphics
	{
		class GPUShaderManager;

		struct ShaderInputLayout
		{
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

			ShaderDesc(std::string vertex, std::string pixel)
				: VertexFilePath(vertex), PixelFilePath(pixel)
			{ }
			ShaderDesc(std::string vertex, std::string pixel, std::vector<ShaderInputLayout> vertexLayout)
				: VertexFilePath(vertex), PixelFilePath(pixel), VertexLayout(vertexLayout)
			{ }

			std::string VertexFilePath = "";
			std::string TesselationControlFilePath = "";
			std::string TesselationEvaluationVertexFilePath = "";
			std::string GeoemtyFilePath = "";
			std::string PixelFilePath = "";
			std::string MainFunc = "main";

			std::vector<ShaderInputLayout> VertexLayout;

			u64 GetHash() const
			{
				u64 hash = 0;

				HashCombine(hash, VertexFilePath);
				HashCombine(hash, TesselationControlFilePath);
				HashCombine(hash, TesselationEvaluationVertexFilePath);
				HashCombine(hash, GeoemtyFilePath);
				HashCombine(hash, PixelFilePath);
				HashCombine(hash, MainFunc);

				for (const auto& input : VertexLayout)
				{
					HashCombine(hash, input.GetHash());
				}

				return hash;
			}

			bool IsValid() const
			{
				return !VertexFilePath.empty() 
					&& !PixelFilePath.empty();
			}
		};

		class GPUShader
		{
		public:
			virtual ~GPUShader() { }

		private:
			virtual void Create(ShaderDesc desc) = 0;
			virtual void Destroy() = 0;

			friend class GPUShaderManager;
		};

		class GPUShaderManager
		{
		public:

			GPUShader* CreateShader(std::string key, ShaderDesc desc);
			GPUShader* GetShader(std::string key);
			void DestroyShader(std::string key);

			void Destroy();

		private:
			std::map<std::string, GPUShader*> m_shaders;
		};
	}
}