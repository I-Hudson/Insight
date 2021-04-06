#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Memory/MemoryUtils.h"
#include "Engine/Memory/SinglyLinkedList.h"

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
			FreeListAllocator(const Size size, const PlacementPolicy policy);
			~FreeListAllocator();

			void* Alloc(const Size size, const Byte TypeSize, const std::string& typeName, const Byte alignment);
			void Free(void* ptr);

			template <typename T, typename... Args>
			T* New(Args... args);
			template <typename T>
			void Delete(T* ptr);
			template <typename T>
			T* NewArr(const Size length, const Byte alignment);
			template <typename T>
			void DeleteArr(T* ptrToDelete);

			void* NewArr(const U64& size, const Byte& alignment);
			void DeleteArr(void* ptrToDelete);

			std::string GetAllocationOfType(void* ptr);

		private:
			const static int AllocHeader_TypeNameLength = 64;
			struct AllocHeader 
			{
				Size BlockSize;
				Byte TypeSize;
				char TypeName[AllocHeader_TypeNameLength];
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
#if defined(IS_DEBUG)
			Size m_numOfNews{ 0 };
			Size m_numOfDeletes{ 0 };
			Size m_numOfArrNews{ 0 };
			Size m_numOfArrDeletes{ 0 };
			Size m_numOfAllocs{ 0 };
			Size m_numOfFrees{ 0 };
			std::unordered_map<U64, std::string> m_vtableToNameMap;
#endif	
			friend class MemoryManager;
		};

		template<typename T, typename ...Args>
		inline T* FreeListAllocator::New(Args... args)
		{
			T* ptr = new (Alloc((Size)sizeof(T), (Byte)sizeof(T), GET_SHORT_NAME_OF_TYPE(T), MemoryUtlis::Alignment)) T(args...);
#if defined(IS_DEBUG)
			m_numOfNews++;
			std::string name = std::string(typeid(T).name());
			U64 vPointer = *reinterpret_cast<U64*>(ptr);
			m_vtableToNameMap.insert({ vPointer, name });
#endif
			return ptr;
		}

		template<typename T>
		inline void FreeListAllocator::Delete(T* ptr)
		{
#if defined(IS_DEBUG)
			m_numOfDeletes++;

			for (auto it = m_vtableToNameMap.begin(); it != m_vtableToNameMap.end(); ++it) 
			{
				U64 vPointer = *reinterpret_cast<U64*>(ptr);
				if ((*it).first == vPointer)
				{
					m_vtableToNameMap.erase(it);
					break;
				}
			}
#endif
			ptr->~T();
			Free(static_cast<void*>(ptr));
		}

		template<typename T>
		inline T* FreeListAllocator::NewArr(const Size length, Byte alignment)
		{
			void* alloc = Alloc(sizeof(T) * length, sizeof(T), typeid(T).name(), alignment);
#if defined(IS_DEBUG)
			m_numOfArrNews++;
			std::string name = std::string(typeid(T).name());
			if (std::is_base_of<class Component, T>::value) 
			{
				U64 vPointer = *reinterpret_cast<U64*>(alloc);
				m_vtableToNameMap.insert({ vPointer, name });
			}
#endif
			char* allocAddress = static_cast<char*>(alloc);
			for (Size i = 0; i < length; ++i) new (allocAddress + i * sizeof(T)) T;
			return static_cast<T*>(alloc);
		}

		template<typename T>
		inline void FreeListAllocator::DeleteArr(T* ptrToDelete)
		{
#if defined(IS_DEBUG)
			m_numOfArrDeletes++;
			std::string name;

			for (auto it = m_vtableToNameMap.begin(); it != m_vtableToNameMap.end(); ++it)
			{
				U64 vPointer = *reinterpret_cast<U64*>(ptrToDelete);
				if ((*it).first == vPointer)
				{
					m_vtableToNameMap.erase(it);
					break;
				}
			}
#endif
			PtrInt headerAddress = (Size)ptrToDelete - sizeof(FreeListAllocator::AllocHeader);
			FreeListAllocator::AllocHeader* allocHeader = (FreeListAllocator::AllocHeader*) headerAddress;
			int length = allocHeader->BlockSize - allocHeader->AlignmentPadding - sizeof(FreeListAllocator::AllocHeader);
			length /= allocHeader->TypeSize;

			for (Size i = 0; i < length; ++i)
			{
				PtrInt object = (PtrInt)ptrToDelete + (allocHeader->TypeSize * i);
				((T*)object)->~T();
			}
			Free(static_cast<void*>(ptrToDelete));
		}
	}