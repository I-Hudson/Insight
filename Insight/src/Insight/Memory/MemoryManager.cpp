#include "ispch.h"

#include "MemoryManager.h"
#include "Insight/Config/Config.h"
#include "Insight/Core/Log.h"
#include <typeinfo>

	namespace Memory
	{
		void MemoryManager::DeleteOnStack(const Size marker)
		{
			return Instance()->m_stackAllocator.FreeToMarker(marker);
		}

		void* MemoryManager::AlignedAlloc(const U64& size, const U64& alignment)
		{
			void* data = nullptr;
#if defined(_MSC_VER) || defined(__MINGW32__)
			data = _aligned_malloc(size, alignment);
#else
			int res = posix_memalign(&data, alignment, size);
			if (res != 0)
				data = nullptr;
#endif
			return data;
		}

		void* MemoryManager::NewArrOnFreeListVoid(const U64& size, U8 alignment)
		{
			return Instance()->m_freeListAllocator.NewArr(size, alignment);
		}

		void MemoryManager::DeleteArrOnFreeListVoid(void* ptrToDelete)
		{
			return Instance()->m_freeListAllocator.DeleteArr(ptrToDelete);
		}

		void MemoryManager::TrackObject(void* ptr, const std::string& str, const std::string& file, const unsigned int& line)
		{
			if (!Instance()->m_trackingObjects.count(ptr))
			{
				Instance()->m_trackingObjects[ptr] = TrackingObjectRecord
				{
					ptr,
					str,
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
			if (Instance()->m_trackingObjects.count(ptr))
			{
				Instance()->m_trackingObjects.erase(ptr);
			}
			else
			{
				IS_CORE_WARN("Object has already being untracked: {0}!", ptr);
			}
		}

		std::string MemoryManager::GetAllocationOfType(void* ptr)
		{
			std::string type;
			type = m_freeListAllocator.GetAllocationOfType(ptr);
			if (type.empty())
			{
				return "Invalid";
			}

			return type;
		}

		MemoryManager::MemoryType MemoryManager::StringToMemoryType(const std::string& string)
		{
			if (string == "B")
			{
				return MemoryType::B;
			}
			else if (string == "KB")
			{
				return MemoryType::KB;
			}
			else if (string == "MB")
			{
				return MemoryType::MB;
			}
			else if (string == "GB")
			{
				return MemoryType::GB;
			}
			else if (string == "TB")
			{
				return MemoryType::TB;
			}

			return MemoryType::MB;
		}

		Size MemoryManager::GetConfigMemorySize(const Size& size, const std::string& type)
		{
			MemoryType memoryType = StringToMemoryType(type);

			if (memoryType == MemoryManager::MemoryType::B)
			{
				return size;
			}
			else
			{
				return static_cast<Size>(size * pow(1024, (int)memoryType));
			}
		}

		MemoryManager::MemoryManager()
#ifndef IS_SMART_POINTERS_IN_USE
			: m_stackAllocator(GetConfigMemorySize(CONFIG_VAL(Config::MemoryConfig.StackAllocAmount), CONFIG_VAL(Config::MemoryConfig.StackAllocType)))
			, m_freeListAllocator(GetConfigMemorySize(CONFIG_VAL(Config::MemoryConfig.FreeListAllocAmount), CONFIG_VAL(Config::MemoryConfig.FreeListAllocType))
				,(Memory::FreeListAllocator::PlacementPolicy)CONFIG_VAL(Config::MemoryConfig.PlacementPolicy))
#endif
		{ }

		MemoryManager::~MemoryManager()
		{
			for (auto it = m_trackingObjects.begin(); it != m_trackingObjects.end(); ++it)
			{
				IS_CORE_ERROR("Tracking {0} has not been destroyed or untracked. => Ptr: {1}, File: {2}, Line: {3}",
					(*it).second.Str, (*it).second.Ptr, (*it).second.File, (*it).second.Line);
			}
		}
	}