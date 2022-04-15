#include "Graphics/RHI/RHI_Shader.h"
#include "Graphics/GraphicsManager.h"

#include "Graphics/RHI/Vulkan/RHI_Shader_Vulkan.h"

namespace Insight
{
	namespace Graphics
	{
		RHI_Shader* RHI_Shader::New()
		{
			if (GraphicsManager::IsVulkan()) { return new RHI::Vulkan::RHI_Shader_Vulkan(); }
			//else if (GraphicsManager::IsDX12()) { return new RHI::DX12::GPUShader_DX12(); }
			return nullptr;
		}


		RHI_ShaderManager::RHI_ShaderManager()
		{
		}

		RHI_ShaderManager::~RHI_ShaderManager()
		{
		}

		RHI_Shader* RHI_ShaderManager::GetOrCreateShader(ShaderDesc desc)
		{
			if (!desc.IsValid())
			{
				return nullptr;
			}
			const u64 hash = desc.GetHash();
			auto itr = m_shaders.find(hash);
			if (itr != m_shaders.end())
			{
				return itr->second;
			}

			RHI_Shader* shader = RHI_Shader::New();
			shader->Create(m_context, desc);
			m_shaders[hash] = shader;

			return shader;
		}

		void RHI_ShaderManager::Destroy()
		{
			for (const auto& pair : m_shaders)
			{
				pair.second->Destroy();
				delete pair.second;
			}
			m_shaders.clear();
		}
	}
}