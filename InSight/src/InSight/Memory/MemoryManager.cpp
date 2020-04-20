#include "ispch.h"

#include "MemoryManager.h"
#include "Insight/Log.h"


namespace Insight
{
	namespace Memory
	{
		MemoryManager* Memory::MemoryManager::s_instance;

		void MemoryManager::DeleteOnStack(const Size marker)
		{
			return GetInstance()->m_stackAllocator.FreeToMarker(marker);
		}

		MemoryManager* MemoryManager::GetInstance()
		{
			if (s_instance != nullptr)
			{
				return s_instance;
			}
			else
			{
				IS_CORE_ERROR("MemoryManager: GetInstance => instnace does not exists.");
			}
			return nullptr;
		}

		MemoryManager::MemoryManager()
			:m_stackAllocator(512_B), m_freeListAllocator(512_MB, Memory::FreeListAllocator::PlacementPolicy::FIND_FIRST)
		{
			s_instance = this;
		}

		MemoryManager::~MemoryManager()
		{
		}
	}
}