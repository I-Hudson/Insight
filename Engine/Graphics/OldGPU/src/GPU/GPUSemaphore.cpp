#include "Graphics/GPU/GPUSemaphore.h"
#include "Core/Logger.h"

#include "Graphics/GPU/RHI/Vulkan/GPUSemaphore_Vulkan.h"
#include "Graphics/GPU/RHI/DX12/GPUSemaphore_DX12.h"

#include <iostream>

namespace Insight
{
	namespace Graphics
	{
		GPUSemaphore* GPUSemaphore::New()
		{
			if (GraphicsManager::IsVulkan()) { return new RHI::Vulkan::GPUSemaphore_Vulkan(); }
			else if (GraphicsManager::IsDX12()) { return new RHI::DX12::GPUSemaphore_DX12(); }
			return nullptr;
		}


		GPUSemaphoreManager::~GPUSemaphoreManager()
		{
			if (m_inUseSemaphroes.size() > 0)
			{
				IS_CORE_ERROR("[GPUSemaphoreManager::~GPUSemaphoreManager] Not all semaphores have been returned.");
			}
		}

		GPUSemaphore* GPUSemaphoreManager::GetOrCreateSemaphore(bool signaled)
		{
			if (!signaled && m_freeSemaphroes.size() > 0)
			{
				GPUSemaphore*& semaphore = m_freeSemaphroes.back();
				m_freeSemaphroes.pop_back();
				m_inUseSemaphroes.push_back(semaphore);
				return semaphore;
			}

			GPUSemaphore* semaphore = GPUSemaphore::New();
			semaphore->Create(signaled);
			m_inUseSemaphroes.push_back(semaphore);
			return semaphore;
		}

		void GPUSemaphoreManager::ReturnSemaphore(GPUSemaphore*& semaphore)
		{
			std::list<GPUSemaphore*>::const_iterator itr = std::find(m_inUseSemaphroes.begin(), m_inUseSemaphroes.end(), semaphore);
			if (itr != m_inUseSemaphroes.end())
			{
				m_inUseSemaphroes.erase(itr);
			}
			else
			{
				IS_CORE_ERROR("[GPUSemaphoreManager::ReturnSemaphore] Semaphore is not tracked.All semaphores should be created using GetDevice()->GetSemaphoreManager().GetOrCreateSemaphore().");
			}

			itr = std::find(m_freeSemaphroes.begin(), m_freeSemaphroes.end(), semaphore);
			if (itr != m_freeSemaphroes.end())
			{
				IS_CORE_ERROR("[GPUSemaphoreManager::ReturnSemaphore] Semaphore already in free list.");
				return;
			}
			m_freeSemaphroes.push_back(semaphore);
			semaphore = nullptr;
		}

		void GPUSemaphoreManager::Destroy()
		{
			if (m_inUseSemaphroes.size() > 0)
			{
				IS_CORE_ERROR("[GPUSemaphoreManager::Destroy] Semaphores in use. Not all semaphores have been returned.");
			}

			for (std::list<GPUSemaphore*>::iterator::value_type& ptr : m_inUseSemaphroes)
			{
				ptr->Destroy();
				delete ptr;
				ptr = nullptr;
			}
			m_inUseSemaphroes.clear();

			for (std::list<GPUSemaphore*>::iterator::value_type& ptr : m_freeSemaphroes)
			{
				ptr->Destroy();
				delete ptr;
				ptr = nullptr;
			}
			m_freeSemaphroes.clear();
		}
	}
}