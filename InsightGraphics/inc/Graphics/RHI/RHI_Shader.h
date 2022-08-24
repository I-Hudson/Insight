#pragma once

#include "Graphics/PixelFormat.h"
#include "Graphics/RHI/RHI_Descriptor.h"
#include "Graphics/Enums.h"
#include "Graphics/ShaderDesc.h"

#include "Graphics/RHI/DX12/RHI_PhysicalDevice_DX12.h"
#include "dxc/dxcapi.h"
#include "spirv_reflect.h"

#include <vector>
#include <map>
#include <unordered_map>

namespace Insight
{
	namespace Graphics
	{
		class RHI_ShaderManager;
		class RenderContext;

		class RHI_Shader
		{
		public:
			virtual ~RHI_Shader() { }

			bool IsCompiled() const { return m_compiled; }
			std::vector<Descriptor> GetDescriptors() const { return m_descriptors; }
			PushConstant GetPushConstant() const { return m_push_constant; }
			int GetShaderInputLayoutStride() const { return m_shaderInputLayputStride; }

		private:
			static RHI_Shader* New();
			virtual void Create(RenderContext* context, ShaderDesc desc) = 0;
			virtual void Destroy() = 0;

		protected:
			bool m_compiled = false;
			std::vector<Descriptor> m_descriptors;
			PushConstant m_push_constant;
			std::vector<ShaderInputLayout> m_shaderInputLayout;
			int m_shaderInputLayputStride = 0;

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

		enum class ShaderCompilerLanguage
		{
			Spirv,
			Hlsl
		};

		struct ShaderCompiler
		{
			ShaderCompiler();
			ShaderCompiler(const ShaderCompiler& other) = delete;
			ShaderCompiler(ShaderCompiler&& other) = delete;
			~ShaderCompiler();

			std::wstring StageToFuncName(ShaderStageFlagBits stage);
			std::wstring StageToProfileTarget(ShaderStageFlagBits stage);

			RHI::DX12::ComPtr<IDxcBlob> Compile(ShaderStageFlagBits stage, std::wstring_view filePath, ShaderCompilerLanguage languageToCompileTo);
			void GetDescriptors(ShaderStageFlagBits stage, std::vector<Descriptor>& descriptors, PushConstant& push_constant);
			std::vector<ShaderInputLayout> GetInputLayout();

			DescriptorType SpvReflectDescriptorTypeToDescriptorType(SpvReflectDescriptorType type);
			DescriptorResourceType SpvReflectDescriptorResourceTypeToDescriptorResourceType(SpvReflectResourceType type);

			ShaderCompilerLanguage m_languageToCompileTo;

			RHI::DX12::ComPtr<IDxcUtils> DXUtils;
			RHI::DX12::ComPtr<IDxcCompiler3> DXCompiler;

			RHI::DX12::ComPtr<IDxcResult> ShaderCompileResults;
			RHI::DX12::ComPtr<IDxcResult> ShaderReflectionResults;
		};
	}
}