#pragma once

#include "Insight/Core.h"

#include "Insight/Config/CVar.h"
#include "Insight/Templates/TSingleton.h"
#include "Insight/InsightAlias.h"
#include "FreeListAllocator.h"
#include "StackAllocator.h"

namespace Insight
{
	namespace Memory
	{
		class IS_API MemoryManager : public TSingleton<MemoryManager>
		{
		public:
			MemoryManager();
			~MemoryManager();

			template <typename T, typename... Args>
			static T* NewOnStack(Args&&...);

			template<typename T, typename... Args>
			static T* NewArrOnStack(Size length, U8 alignment = MemoryUtlis::Alignment);

			static void DeleteOnStack(const Size marker);

			template<typename T, typename... Args>
			static T* NewOnFreeList(Args&& ...);
			template<typename T>
			static void DeleteOnFreeList(T* ptrToDelete);

			template <typename T>
			static T* NewArrOnFreeList(Size length, U8 alignment = MemoryUtlis::Alignment);
			template<typename T>
			static void DeleteArrOnFreeList(T* ptrToDelete);

			static void TrackObject(void* ptr, const std::string& str, const std::string& file, const unsigned int& line);
			static void UnTrackObject(void* ptr);

			std::string GetAllocationOfType(void* ptr);

			static void PrintStackAllocatorUsed() { Instance()->m_stackAllocator.PrintUsed(); }

			struct MemoryConfig
			{
				CVar<unsigned int>	StackAllocAmount		{ "stack_alloc_amount", 512 };
				CVarString			StackAllocType			{ "stack_alloc_type", "KB" };
				CVar<unsigned int>	FreeListAllocAmount		{ "free_list_alloc_amount", 512 };
				CVarString			FreeListAllocType		{ "free_list_alloc_type", "KB" };
				CVar<unsigned int>	PlacementPolicy			{ "placement_policy", 0 };
			};

		private:

			enum MemoryType { B, KB, MB, GB, TB };

			MemoryType StringToMemoryType(const std::string& string);
			Size GetConfigMemorySize(const Size& size, const std::string& type);

			struct TrackingObjectRecord
			{
				void* Ptr;
				std::string Str;
				std::string File;
				unsigned int Line;
			};

			std::unordered_map<void*, TrackingObjectRecord> m_trackingObjects;
			StackAllocator m_stackAllocator;
			FreeListAllocator m_freeListAllocator;
			bool m_smartPointersInUse;
		};

		template<typename T, typename ...Args>
		inline T* MemoryManager::NewOnStack(Args&&... argList)
		{
			return Instance()->m_stackAllocator.New<T>(std::forward<Args>(argList)...);
		}

		template<typename T, typename ...Args>
		inline T* MemoryManager::NewArrOnStack(Size length, U8 alignment)
		{
			return Instance()->m_stackAllocator.NewArr<T>(length, alignment);
		}

		template<typename T, typename ...Args>
		inline T* MemoryManager::NewOnFreeList(Args&& ... argList)
		{
			return Instance()->m_freeListAllocator.New<T>(std::forward<Args>(argList)...);
		}

		template<typename T>
		inline void MemoryManager::DeleteOnFreeList(T* ptrToDelete)
		{
			Instance()->m_freeListAllocator.Delete<T>(ptrToDelete);
		}

		template<typename T>
		inline T* MemoryManager::NewArrOnFreeList(Size length, U8 alignment)
		{
			return Instance()->m_freeListAllocator.NewArr<T>(length, alignment);
		}

		template<typename T>
		inline void MemoryManager::DeleteArrOnFreeList(T* ptrToDelete)
		{
			Instance()->m_freeListAllocator.DeleteArr<T>(ptrToDelete);
		}
	}
}

