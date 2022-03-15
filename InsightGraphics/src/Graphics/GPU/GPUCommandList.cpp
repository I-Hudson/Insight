#include "Graphics/GPU/GPUCommandList.h"
#include "Graphics/GPU/RHI/Vulkan/GPUCommandList_Vulkan.h"

#include <iostream>

namespace Insight
{
	namespace Graphics
	{
		void GPUCommandList::Submit(GPUQueue queue)
		{
			Submit(queue, {}, {}, nullptr);
		}

		void GPUCommandList::SubmitAndWait(GPUQueue queue)
		{
			SubmitAndWait(queue, {}, {}, nullptr);
		}


		GPUCommandListManager::GPUCommandListManager()
		{
		}

		GPUCommandListManager::~GPUCommandListManager()
		{
			Destroy();
		}

		void GPUCommandListManager::Create()
		{
			if (!m_commandListAllocator)
			{
				m_commandListAllocator = new RHI::Vulkan::GPUComamndListAllocator_Vulkan();
			}
		}

		void GPUCommandListManager::SetQueue(GPUQueue queue)
		{
			m_commandListAllocator->SetQueue(queue);
		}

		void GPUCommandListManager::ResetCommandList(GPUCommandList* cmdList)
		{
			ResetCommandLists({cmdList});
		}

		void GPUCommandListManager::ResetCommandLists(std::list<GPUCommandList*> cmdLists)
		{
			m_commandListAllocator->ResetCommandLists(cmdLists);
		}

		void GPUCommandListManager::ResetCommandPool(GPUCommandListType type)
		{
			m_commandListAllocator->ResetCommandPool(type);
		}

		GPUCommandList* GPUCommandListManager::GetOrCreateCommandList(GPUCommandListType type)
		{
			if (m_freeCommandLists.size() > 0)
			{
				GPUCommandList* cmdList = m_freeCommandLists.back();
				m_freeCommandLists.pop_back();
				m_inUseCommandLists.push_back(cmdList);
				return cmdList;
			}

			GPUCommandList* cmdList = m_commandListAllocator->AllocateCommandList(type);
			m_inUseCommandLists.push_back(cmdList);
			return cmdList;
		}

		void GPUCommandListManager::ReturnCommandList(GPUCommandList* cmdList)
		{
			std::list<GPUCommandList*>::iterator itr = std::find(m_inUseCommandLists.begin(), m_inUseCommandLists.end(), cmdList);
			if (itr != m_inUseCommandLists.end())
			{
				m_inUseCommandLists.erase(itr);
			}
			else
			{
				// ERROR: Command list not being tracked.
			}

			itr = std::find(m_freeCommandLists.begin(), m_freeCommandLists.end(), cmdList);
			if (itr != m_freeCommandLists.end())
			{
				// ERROR: Command list already in free list.
				return;
			}
			m_freeCommandLists.push_back(cmdList);
		}

		void GPUCommandListManager::Destroy()
		{
			if (m_inUseCommandLists.size() > 0)
			{
				// ERROR: Command lists are in use.
			}
			m_commandListAllocator->FreeCommandLists(m_inUseCommandLists);
			m_commandListAllocator->FreeCommandLists(m_freeCommandLists);
			m_inUseCommandLists.resize(0);
			m_freeCommandLists.resize(0);
		}
	}
}