#include "Graphics/GPU/GPUBuffer.h"
#include "Graphics/GraphicsManager.h"
#include "Graphics/GPU/RHI/Vulkan/GPUBuffer_Vulkan.h"
#include "Core/Logger.h"

#include <iostream>

namespace Insight
{
	namespace Graphics
	{
		GPUBuffer* GPUBuffer::New()
		{
			if (GraphicsManager::IsVulkan()) { return new RHI::Vulkan::GPUBuffer_Vulkan(); }
			else if (GraphicsManager::IsDX12()) { return nullptr; }// new RHI::DX12::GPUDevice_Buffer(); }
			return nullptr;
		}


		GPUBufferManager::GPUBufferManager()
		{
		}

		GPUBufferManager::~GPUBufferManager()
		{
			if (m_buffers.size() > 0)
			{
				IS_CORE_ERROR("[GPUBufferManager::~GPUBufferManager] Buffers have not been destroyed.");
			}
		}

		GPUBuffer* GPUBufferManager::CreateBuffer(std::string key, GPUBufferCreateInfo createInfo)
		{
			GPUBuffer* buffer = GetBuffer(key);
			if (buffer != nullptr)
			{
				IS_CORE_ERROR("[GPUBufferManager::CreateBuffer] Buffer exists with key: {}.", key);
				return buffer;
			}


			buffer = GPUBuffer::New();
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
			IS_CORE_ERROR("[GPUBufferManager::DestroyBuffer] Buffer not being tracked. Buffer must be created through GPUBufferManager.");
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