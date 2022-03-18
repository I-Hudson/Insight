#include "Graphics/GPU/GPUFence.h"
#include "Graphics/GPU/RHI/Vulkan/GPUFence_Vulkan.h"

#include <iostream>

namespace Insight
{
	namespace Graphics
	{
		GPUFenceManager::GPUFenceManager()
		{
		}

		GPUFenceManager::~GPUFenceManager()
		{
			if (m_inUseFences.size() > 0 || m_freeFences.size() > 0)
			{
				std::cout << "[GPUFenceManager::~GPUFenceManager] Destroy must be called.\n";
			}
		}

		GPUFence* GPUFenceManager::GetFence()
		{
			if (m_freeFences.size() > 0)
			{
				GPUFence* fence = m_freeFences.back();
				m_freeFences.pop_back();
				m_inUseFences.push_back(fence);
				return fence;
			}

			GPUFence* fence = new RHI::Vulkan::GPUFence_Vulkan();
			fence->Create();
			m_inUseFences.push_back(fence);
			return fence;
		}

		void GPUFenceManager::ReturnFence(GPUFence* fence)
		{
			auto itr = std::find(m_inUseFences.begin(), m_inUseFences.end(), fence);
			if (itr != m_inUseFences.end())
			{
				m_inUseFences.erase(itr);
			}
			else
			{
				std::cout << "[GPUFenceManager::ReturnFence] Fence is not being tracked. Fences must be create through GPUFenceManager.\n";
			}

			itr = std::find(m_freeFences.begin(), m_freeFences.end(), fence);
			if (itr != m_freeFences.end())
			{
				std::cout << "[GPUFenceManager::ReturnFence] Fence is already in the free list.\n";
				return;
			}
			m_freeFences.push_back(fence);
			fence->Reset();
		}

		void GPUFenceManager::Destroy()
		{
			for (const auto& itr : m_inUseFences)
			{
				itr->Destroy();
				delete itr;
			}
			m_inUseFences.resize(0);

			for (const auto& itr : m_freeFences)
			{
				itr->Destroy();
				delete itr;
			}
			m_freeFences.resize(0);
		}
	}
}