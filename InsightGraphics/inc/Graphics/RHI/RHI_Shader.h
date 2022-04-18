#pragma once

#include "Graphics/PixelFormat.h"
#include "Graphics/RHI/RHI_Descriptor.h"
#include "Graphics/Enums.h"
#include <string>
#include <vector>
#include <map>
#include <unordered_map>

#include "Graphics/RHI/DX12/RHI_PhysicalDevice_DX12.h"
#include "dxc/dxcapi.h"

namespace Insight
{
	namespace Graphics
	{
		class RHI_ShaderManager;
		class RenderContext;

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
			ShaderDesc(std::wstring vertex, std::wstring pixel, std::vector<ShaderInputLayout> vertexLayout)
				: VertexFilePath(vertex), PixelFilePath(pixel), VertexLayout(vertexLayout)
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

		class RHI_Shader
		{
		public:
			virtual ~RHI_Shader() { }

			bool IsCompiled() const { return m_compiled; }

		private:
			static RHI_Shader* New();
			virtual void Create(RenderContext* context, ShaderDesc desc) = 0;
			virtual void Destroy() = 0;

		protected:
			bool m_compiled = false;
			std::unordered_map<int, std::vector<Descriptor>> m_descriptors;

			friend RHI_ShaderManager;
		};

		class RHI_ShaderManager
		{
		public:
			RHI_ShaderManager();
			~RHI_ShaderManager();

			void SetRenderContext(RenderContext* context) { m_context = context; }
			RHI_Shader* GetOrCreateShader(ShaderDesc desc);
			void Destroy();

		private:
			std::map<u64, RHI_Shader*> m_shaders;
			RenderContext* m_context{ nullptr };
		};

		struct ShaderCompiler
		{
			ShaderCompiler();
			ShaderCompiler(const ShaderCompiler& other) = delete;
			ShaderCompiler(ShaderCompiler&& other) = delete;
			~ShaderCompiler();

			std::wstring StageToFuncName(ShaderStageFlagBits stage);
			std::wstring StageToProfileTarget(ShaderStageFlagBits stage);

			RHI::DX12::ComPtr<IDxcBlob> Compile(ShaderStageFlagBits stage, std::wstring_view filePath, std::vector<std::wstring> additionalArgs = {});
			std::unordered_map<int, std::vector<Descriptor>> GetDescriptors();

			RHI::DX12::ComPtr<IDxcUtils> s_dxUtils;
			RHI::DX12::ComPtr<IDxcCompiler3> s_dxCompiler;
			RHI::DX12::ComPtr<IDxcResult> m_results;
		};
	}
}