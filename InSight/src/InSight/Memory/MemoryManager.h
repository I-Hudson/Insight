#pragma once

#include "Insight/Core.h"

#include "Insight/InsightAlias.h"
#include "FreeListAllocator.h"
#include "StackAllocator.h"

namespace Insight
{
	namespace Memory
	{
		class IS_API MemoryManager
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
			template <typename T>
			static void DeleteArrOnFreeList(Size length, T* ptrToDelete);

			static void TrackObject(void* ptr);
			static void UnTrackObject(void* ptr);

			static void PrintStackAllocatorUsed() { GetInstance()->m_stackAllocator.PrintUsed(); }

			static MemoryManager* GetInstance();

		private:

			static MemoryManager* s_instance;
			std::set<void*> m_trackingObjects;
			StackAllocator m_stackAllocator;
			FreeListAllocator m_freeListAllocator;
		};

		template<typename T, typename ...Args>
		inline T* MemoryManager::NewOnStack(Args&&... argList)
		{
			return GetInstance()->m_stackAllocator.New<T>(std::forward<Args>(argList)...);
		}

		template<typename T, typename ...Args>
		inline T* MemoryManager::NewArrOnStack(Size length, U8 alignment)
		{
			return GetInstance()->m_stackAllocator.NewArr<T>(length, alignment);
		}

		template<typename T, typename ...Args>
		inline T* MemoryManager::NewOnFreeList(Args&& ... argList)
		{
			return GetInstance()->m_freeListAllocator.New<T>(std::forward<Args>(argList)...);
		}

		template<typename T>
		inline void MemoryManager::DeleteOnFreeList(T* ptrToDelete)
		{
			GetInstance()->m_freeListAllocator.Delete<T>(ptrToDelete);
		}

		template<typename T>
		inline T* MemoryManager::NewArrOnFreeList(Size length, U8 alignment)
		{
			return GetInstance()->m_freeListAllocator.NewArr<T>(length, alignment);
		}

		template<typename T>
		inline void MemoryManager::DeleteArrOnFreeList(Size length, T* ptrToDelete)
		{
			GetInstance()->m_freeListAllocator.DeleteArr<T>(length, ptrToDelete);
		}
	}
}

