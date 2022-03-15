#include "Graphics/GPU/GPUSemaphore.h"
#include "Graphics/GPU/RHI/Vulkan/GPUSemaphore_Vulkan.h"
#include <iostream>

namespace Insight
{
	namespace Graphics
	{
		GPUSemaphoreManager::GPUSemaphoreManager()
		{
		}

		GPUSemaphoreManager::~GPUSemaphoreManager()
		{
			Destroy();
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

			GPUSemaphore* semaphore = new RHI::Vulkan::GPUSemaphore_Vulkan();
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
				std::cout << "[GPUSemaphoreManager::ReturnSemaphore] Semaphore: " << semaphore << " is not tracked. All semaphores should be created using GetDevice()->GetSemaphoreManager().GetOrCreateSemaphore().\n";
			}

			itr = std::find(m_freeSemaphroes.begin(), m_freeSemaphroes.end(), semaphore);
			if (itr != m_freeSemaphroes.end())
			{
				std::cout << "[GPUSemaphoreManager::ReturnSemaphore] Semaphore: "<< semaphore << " already in free list.\n";
				return;
			}
			m_freeSemaphroes.push_back(semaphore);
			semaphore = nullptr;
		}

		void GPUSemaphoreManager::Destroy()
		{
			if (m_inUseSemaphroes.size() > 0)
			{
				std::cout << "[GPUSemaphoreManager::Destroy] Semaphores in use. Not all semaphores have been returned.\n";
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