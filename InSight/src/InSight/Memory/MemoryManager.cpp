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

		void MemoryManager::TrackObject(void* ptr)
		{
			if (!GetInstance()->m_trackingObjects.count(ptr))
			{
				GetInstance()->m_trackingObjects.insert(ptr);
			}
			else
			{
				IS_CORE_ERROR("Object is already being tracked: {0}!", ptr);
			}
		}

		void MemoryManager::UnTrackObject(void* ptr)
		{
			if (GetInstance()->m_trackingObjects.count(ptr))
			{
				GetInstance()->m_trackingObjects.erase(ptr);
			}
			else
			{
				IS_CORE_ERROR("Object has already being untracked: {0}!", ptr);
			}
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
			:m_stackAllocator(512_KB), m_freeListAllocator(512_KB, Memory::FreeListAllocator::PlacementPolicy::FIND_FIRST)
		{
			s_instance = this;
		}

		MemoryManager::~MemoryManager()
		{
			for (auto it = m_trackingObjects.begin(); it != m_trackingObjects.end(); ++it)
			{
				IS_CORE_ERROR("Tracking object has not been destroyed {0}", (*it));
			}
		}
	}
}