#include "Graphics/GPU/GPUBuffer.h"
#include "Graphics/GPU/RHI/Vulkan/GPUBuffer_Vulkan.h"

#include <iostream>

namespace Insight
{
	namespace Graphics
	{
		GPUBufferManager::GPUBufferManager()
		{
		}

		GPUBufferManager::~GPUBufferManager()
		{
			if (m_buffers.size() > 0)
			{
				std::cout << "[GPUBufferManager::~GPUBufferManager] Destroy must be called.\n";
			}
		}

		GPUBuffer* GPUBufferManager::CreateBuffer(std::string key, GPUBufferCreateInfo createInfo)
		{
			GPUBuffer* buffer = GetBuffer(key);
			if (buffer != nullptr)
			{
				std::cout << "[GPUBufferManager::CreateBuffer] Buffer exists with key: " << key << "\n";
				return buffer;
			}

			buffer = new RHI::Vulkan::GPUBuffer_Vulkan();
			buffer->Create(createInfo);
			m_buffers[key] = buffer;
			return buffer;
		}

		GPUBuffer* GPUBufferManager::GetBuffer(std::string key)
		{
			auto itr = m_buffers.find(key);
			if (itr != m_buffers.end())
			{
				return itr->second;
			}
			return nullptr;
		}

		void GPUBufferManager::DestroyBuffer(std::string key)
		{
			auto itr = m_buffers.find(key);
			if (itr == m_buffers.end())
			{
				return;
			}
			itr->second->Destroy();
			delete itr->second;
			m_buffers.erase(itr);
		}

		void GPUBufferManager::DestroyBuffer(GPUBuffer* buffer)
		{
			for (const auto& pair : m_buffers)
			{
				if (pair.second == buffer)
				{
					DestroyBuffer(pair.first);
					return;
				}
			}
			std::cout << "[GPUBufferManager::DestroyBuffer] Buffer not being tracked. Buffer must be created through GPUBufferManager.\n";
		}

		void GPUBufferManager::Destroy()
		{
			for (const auto& pair : m_buffers)
			{
				pair.second->Destroy();
				delete pair.second;
			}
			m_buffers.clear();
		}
	}
}