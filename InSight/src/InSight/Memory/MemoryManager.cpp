#include "ispch.h"

#include "MemoryManager.h"
#include "Insight/Config/Config.h"
#include "Insight/Log.h"
#include <typeinfo>

namespace Insight
{
	namespace Memory
	{
		void MemoryManager::DeleteOnStack(const Size marker)
		{
			return GetInstance()->m_stackAllocator.FreeToMarker(marker);
		}

		void MemoryManager::TrackObject(void* ptr, const std::string& file, const const unsigned int& line)
		{
			if (!GetInstance()->m_trackingObjects.count(ptr))
			{
				GetInstance()->m_trackingObjects[ptr] = TrackingObjectRecord
				{
					ptr,
					file,
					line
				};
			}
			else
			{
				IS_CORE_WARN("Object is already being tracked: {0}!", ptr);
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
				IS_CORE_WARN("Object has already being untracked: {0}!", ptr);
			}
		}

		Size MemoryManager::GetConfigMemorySize(const Size& size, const std::string& type)
		{
			const unsigned int KBSize = 1024;


			return size * pow(KBSize, 1);
		}

		void MemoryManager::ClearTypeInfoCache()
		{
			while (auto entry = InterlockedPopEntrySList(reinterpret_cast<PSLIST_HEADER>(&__type_info_root_node)))
			{
				free(entry);
			}
		}

		MemoryManager::MemoryManager()
#ifndef IS_SMART_POINTERS_IN_USE
			:m_stackAllocator(GetConfigMemorySize(CONFIG_VAL(Config::MemoryConfig.StackAllocAmount), CONFIG_VAL(Config::MemoryConfig.StackAllocType)))
			, m_freeListAllocator(GetConfigMemorySize(CONFIG_VAL(Config::MemoryConfig.FreeListAllocAmount), CONFIG_VAL(Config::MemoryConfig.FreeListAllocType))
				,(Insight::Memory::FreeListAllocator::PlacementPolicy)CONFIG_VAL(Config::MemoryConfig.PlacementPolicy))
#endif
		{ }

		MemoryManager::~MemoryManager()
		{
			for (auto it = m_trackingObjects.begin(); it != m_trackingObjects.end(); ++it)
			{
				IS_CORE_ERROR("Tracking object has not been destroyed or untracked. => Ptr: {0}, File: {1}, Line: {2}", 
					(*it).second.Ptr, (*it).second.File, (*it).second.Line);
			}

			ClearTypeInfoCache();
		}
	}
}