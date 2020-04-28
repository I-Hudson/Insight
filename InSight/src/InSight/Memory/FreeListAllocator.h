#pragma once

#include "Insight/Core.h"
#include "Insight/InsightAlias.h"
#include "Insight/Memory/MemoryUtils.h"
#include "Insight/Memory/SinglyLinkedList.h"

namespace Insight
{
	namespace Memory
	{
		class IS_API FreeListAllocator
		{
		public:

			enum PlacementPolicy 
			{
				FIND_FIRST,
				FIND_BEST
			};

			FreeListAllocator() = delete;
			explicit FreeListAllocator(const Size size, const PlacementPolicy policy);
			~FreeListAllocator();

			void* Alloc(const Size size, const Byte alignment);
			void Free(void* ptr);

			template <typename T, typename... Args>
			T* New(Args... args);
			template <typename T>
			void Delete(T* ptr);
			template <typename T>
			T* NewArr(const Size length, const Byte alignment);
			template <typename T>
			void DeleteArr(const Size length, T* ptrToDelete);

		private:
			struct AllocHeader 
			{
				Size BlockSize;
				Byte AlignmentPadding;
			};

			typedef SinglyLinkedList<AllocHeader>::Node Node;

			void* m_startPtr;
			PtrInt m_startAddress;
			PlacementPolicy m_policy;
			SinglyLinkedList<AllocHeader> m_freeList;

			void Coalescence(Node* previousNode, Node* freeNode);

			void Find(const Size size, const Byte alignment, Size& padding, Node*& previousNode, Node*& foundNode);
			void FindBest(const Size size, const Byte alignment, Size& padding, Node*& previousNode, Node*& foundNode);
			void FindFirst(const Size size, const Byte alignment, Size& padding, Node*& previousNode, Node*& foundNode);

			void Reset();

			Size m_totalSize;
			Size m_sizeUsed;
#if _DEBUG
			Size m_numOfNews{ 0 };
			Size m_numOfDeletes{ 0 };
			Size m_numOfArrNews{ 0 };
			Size m_numOfArrDeletes{ 0 };
			Size m_numOfAllocs{ 0 };
			Size m_numOfFrees{ 0 };
			std::unordered_map<U64, std::string> m_vtableToNameMap;
			bool m_monitorPureAlloc = true;
#endif	
			friend class MemoryManager;
		};

		template<typename T, typename ...Args>
		inline T* FreeListAllocator::New(Args... args)
		{
#if _DEBUG
			m_numOfNews++;
			m_monitorPureAlloc = false;
			T* ret = new (Alloc(sizeof(T), MemoryUtlis::Alignment)) T(args...);
			m_monitorPureAlloc = true;
			std::string name = typeid(T).name();
			//if (std::is_base_of<class std:, T>::value)
			{
				U64 vPointer = *reinterpret_cast<U64*>(ret);
				m_vtableToNameMap.insert({ vPointer, name });
			}

			return ret;
#else
			return new (Alloc(sizeof(T), MemoryUtlis::Alignment)) T(args...);
#endif
		}

		template<typename T>
		inline void FreeListAllocator::Delete(T* ptr)
		{
#if _DEBUG
			m_numOfDeletes++;
			std::string name;

			if (std::is_base_of<class Component, T>::value) 
			{
				U64 vPointer = *reinterpret_cast<U64*>(ptr);
				name = m_vtableToNameMap.find(vPointer)->second;
			}
			else 
			{
				name = typeid(T).name();
			}

			ptr->~T();

			m_monitorPureAlloc = false;
			Free(static_cast<void*>(ptr));
			m_monitorPureAlloc = true;
#else
			ptr->~T();
			Free(static_cast<void*>(ptr));
#endif
		}

		template<typename T>
		inline T* FreeListAllocator::NewArr(const Size length, Byte alignment)
		{
#if _DEBUG
			m_monitorPureAlloc = false;
			void* alloc = Alloc(sizeof(T) * length, alignment);
			m_monitorPureAlloc = true;

			m_numOfArrNews++;
			std::string name = typeid(T).name();
			if (std::is_base_of<class Component, T>::value) 
			{
				U64 vPointer = *reinterpret_cast<U64*>(alloc);
				m_vtableToNameMap.insert({ vPointer, name });
			}

			name += " Array[";
			name += std::to_string(length);
			name += "]";
#else
			void* alloc = Alloc(sizeof(T) * length, alignment);
#endif

			char* allocAddress = static_cast<char*>(alloc);
			for (Size i = 0; i < length; ++i) new (allocAddress + i * sizeof(T)) T;
			return static_cast<T*>(alloc);
		}

		template<typename T>
		inline void FreeListAllocator::DeleteArr(const Size length, T* ptrToDelete)
		{
#if _DEBUG
			m_numOfArrDeletes++;
			std::string name;

			if (std::is_base_of<class Component, T>::value) {
				U64 vPointer = *reinterpret_cast<U64*>(ptrToDelete);
				name = m_vtableToNameMap.find(vPointer)->second;
			}
			else 
			{
				name = typeid(T).name();
			}

			name += " Array[";
			name += std::to_string(length);
			name += "]";

			m_monitorPureAlloc = false;
			Free(static_cast<void*>(ptrToDelete));
			m_monitorPureAlloc = true;
#else
			for (Size i = 0; i < length; ++i) ptrToDelete[i].~T();
			Free(static_cast<void*>(ptrToDelete));
#endif
		}
	}
}