#include "Graphics/GPU/GPUShader.h"
#include "Graphics/GPU/RHI/Vulkan/GPUShader_Vulkan.h"
#include "Core/Logger.h"

#include <iostream>

namespace Insight
{
	namespace Graphics
	{
		GPUShader* GPUShader::New()
		{
			if (GraphicsManager::IsVulkan()) { return new RHI::Vulkan::GPUShader_Vulkan(); }
			//else if (GraphicsManager::IsDX12()) { return new RHI::DX12::GPUShader_DX12(); }
			return nullptr;
		}


		GPUShaderManager::~GPUShaderManager()
		{
			if (m_shaders.size() > 0)
			{
				IS_CORE_ERROR("[GPUShaderManager::~GPUShaderManager] Not all shader have been destroyed.");
			}
		}

		GPUShader* GPUShaderManager::CreateShader(std::string key, ShaderDesc desc)
		{
			const auto& itr = m_shaders.find(key);
			if (itr != m_shaders.end())
			{
				IS_CORE_ERROR("[GPUShaderManager::CreateShader] Found already created shader with key: {}.", key);
				return itr->second;
			}

			GPUShader* shader = GPUShader::New();
			shader->Create(desc);
			m_shaders[key] = shader;
			return shader;
		}

		GPUShader* GPUShaderManager::GetShader(std::string key)
		{
			const auto& itr = m_shaders.find(key);
			if (itr == m_shaders.end())
			{
				IS_CORE_ERROR("[GPUShaderManager::GetShader] No shader with key: {}.",key);
				return nullptr;
			}
			return itr->second;
		}

		void GPUShaderManager::DestroyShader(std::string key)
		{
			const auto& itr = m_shaders.find(key);
			if (itr == m_shaders.end())
			{
				IS_CORE_ERROR("[GPUShaderManager::DestroyShader] No shader with key: {} found.", key);
				return;
			}
			itr->second->Destroy();
			delete itr->second;
			m_shaders.erase(itr);
		}

		void GPUShaderManager::Destroy()
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