#include "ispch.h"
#include "Engine/Memory/FreeListAllocator.h"
#include "Engine/Memory/MemoryUtils.h"

#include "Engine/Core/Log.h"

#include <inttypes.h>

	namespace Memory
	{
		FreeListAllocator::FreeListAllocator(const Size size, const PlacementPolicy policy)
			: m_sizeUsed(0), m_totalSize(size), m_policy(policy)
		{
			m_startPtr = malloc(size);
			memset(m_startPtr, 0, size);
			m_startAddress = reinterpret_cast<PtrInt>(m_startPtr);
			Reset();
			m_totalSize += size;
		}

		FreeListAllocator::~FreeListAllocator()
		{

#if defined(IS_DEBUG)
			if (m_sizeUsed > 0)
			{
				IS_CORE_WARN(
					"You did {0} news and {1} deletes; {2} newArrs and {3} "
					"deleteArrs {4} allocs and {5} frees.",
					m_numOfNews, m_numOfDeletes, m_numOfArrNews, m_numOfArrDeletes,
					m_numOfAllocs, m_numOfFrees);

				IS_CORE_WARN(
					"Memory leak of {0} detected on freelist", m_sizeUsed);

				IS_CORE_WARN( 
					"Dumping Memory Leaks Below ");

				for (auto v : m_vtableToNameMap)
				{
					IS_CORE_WARN("Type: {0}", v.second);
				}

				IS_CORE_WARN(
					"See Memory Leak Dump Above ");
			}
			else
			{
				IS_CORE_INFO("NO MEMORY LEAK!");
			}
#endif
			free(m_startPtr);
			m_startPtr = nullptr;

			m_startAddress = 0;
		}

		void* FreeListAllocator::Alloc(const Size size, const Byte TypeSize, const std::string& typeName, const Byte alignment)
		{
			const Size allocationHeaderSize = sizeof(FreeListAllocator::AllocHeader);
			IS_CORE_ASSERT(alignment >= 8, "Alignment must be 8 or greater");

			// Search through the free list for a free block that has enough space to allocate our data
			Size padding;
			Node* affectedNode, * previousNode;
			Find(size, alignment, padding, previousNode, affectedNode);
			IS_CORE_ASSERT(affectedNode != nullptr, "Not enough memory");

			const Byte alignmentPadding = static_cast<Byte>(padding - allocationHeaderSize);
			const Size requiredSize = size + padding;

			const Size rest = affectedNode->data.BlockSize - requiredSize;

			if (rest > sizeof(FreeListAllocator::AllocHeader)) 
			{
				// We have to split the block into the data block and a free block of size 'rest'
				Node* newFreeNode = (Node*)((Size)affectedNode + requiredSize);
				newFreeNode->data.BlockSize = rest;
				newFreeNode->data.AlignmentPadding = 0;
				m_freeList.Insert(affectedNode, newFreeNode);
			}
			m_freeList.Remove(previousNode, affectedNode);

			// Setup data block
			PtrInt headerAddress = (Size)affectedNode + alignmentPadding;
			PtrInt dataAddress = headerAddress + allocationHeaderSize;

			FreeListAllocator::AllocHeader* allocHeader = (FreeListAllocator::AllocHeader*) headerAddress;
			PtrInt paddingAddress = (PtrInt)&allocHeader->AlignmentPadding;
			allocHeader->BlockSize = requiredSize;
			allocHeader->TypeSize = TypeSize;
			IS_CORE_ASSERT(typeName.length() <= AllocHeader_TypeNameLength, "[FreeListAllocator::Alloc] 'type' is longer then allowed. Shortten the 'type' name in this function.");
			strcpy_s(&allocHeader->TypeName[0], typeName.size() * sizeof(char), typeName.c_str());
			allocHeader->AlignmentPadding = alignmentPadding;

			m_sizeUsed += requiredSize + alignmentPadding;

#if defined(IS_DEBUG)
			IS_CORE_INFO("\t@H {0} \tD@ {1} \tS {2} \tAP {3} \tP {4}, \tU {5} \tR {6}", (void*)headerAddress, (void*)dataAddress, ((FreeListAllocator::AllocHeader*) headerAddress)->BlockSize,
				alignmentPadding, padding, m_sizeUsed, rest);
#endif
			return (void*)dataAddress;
		}

		void FreeListAllocator::Free(void* ptr)
		{
			// Insert it in a sorted position by the address number
			const Size currentAddress = (Size)ptr;
			const Size headerAddress = currentAddress - sizeof(FreeListAllocator::AllocHeader);
			const FreeListAllocator::AllocHeader* allocationHeader{ (FreeListAllocator::AllocHeader*) headerAddress };

			Node* freeNode = (Node*)(headerAddress);
			freeNode->data.BlockSize = allocationHeader->BlockSize + allocationHeader->AlignmentPadding;
			freeNode->next = nullptr;

			Node* it = m_freeList.Head;
			Node* itPrev = nullptr;
			while (it != nullptr)
			{
				if (ptr < it)
				{
					m_freeList.Insert(itPrev, freeNode);
					break;
				}
				itPrev = it;
				it = it->next;
			}

			m_sizeUsed -= freeNode->data.BlockSize;

			// Merge contiguous nodes
			Coalescence(itPrev, freeNode);
#if defined(IS_DEBUG)
			m_numOfFrees++;
#endif
		}

		void* FreeListAllocator::NewArr(const u64& size, const Byte& alignment)
		{
			void* alloc = Alloc(size, 0, "void", alignment);
#if defined(IS_DEBUG)
			m_numOfArrNews++;
			std::string name = std::string("void");
			u64 vPointer = *reinterpret_cast<u64*>(alloc);
			m_vtableToNameMap.insert({ vPointer, name });
#endif
			return alloc;
		}

		void FreeListAllocator::DeleteArr(void* ptrToDelete)
		{
#if defined(IS_DEBUG)
			m_numOfArrDeletes++;
			std::string name;

			for (auto it = m_vtableToNameMap.begin(); it != m_vtableToNameMap.end(); ++it)
			{
				u64 vPointer = *reinterpret_cast<u64*>(ptrToDelete);
				if ((*it).first == vPointer)
				{
					m_vtableToNameMap.erase(it);
					break;
				}
			}
#endif
			Free(static_cast<void*>(ptrToDelete));
		}

		std::string FreeListAllocator::GetAllocationOfType(void* ptr)
		{
			const Size currentAddress = (Size)ptr;
			const Size headerAddress = currentAddress - sizeof(FreeListAllocator::AllocHeader);
			const FreeListAllocator::AllocHeader* header{ (FreeListAllocator::AllocHeader*)headerAddress };

			if (header != nullptr)
			{
				return header->TypeName;
			}

			return std::string();
		}

		void FreeListAllocator::Coalescence(Node* previousNode, Node* freeNode)
		{
			if (freeNode->next != nullptr &&
				(Size)freeNode + freeNode->data.BlockSize == (Size)freeNode->next) 
			{
				freeNode->data.BlockSize += freeNode->next->data.BlockSize;
				m_freeList.Remove(freeNode, freeNode->next);
#if defined(IS_DEBUG)
				IS_CORE_INFO("\tMerging(n) {0} % {1} \ts {2}", (void*)freeNode, (void*)freeNode->next, freeNode->data.BlockSize);
#endif
			}

			if (previousNode != nullptr &&
				(Size)previousNode + previousNode->data.BlockSize == (Size)freeNode) 
			{
				previousNode->data.BlockSize += freeNode->data.BlockSize;
				m_freeList.Remove(previousNode, freeNode);
#if defined(IS_DEBUG)
				IS_CORE_INFO("\tMerging(p) {0} & {1} \ts {2}", (void*)previousNode, (void*)freeNode, previousNode->data.BlockSize);
#endif
			}
		}

		void FreeListAllocator::Find(const Size size, const Byte alignment, Size& padding, Node*& previousNode, Node*& foundNode)
		{
			switch (m_policy)
			{
			case PlacementPolicy::FIND_FIRST:
				FindFirst(size, alignment, padding, previousNode, foundNode);
				break;
			case PlacementPolicy::FIND_BEST:
				FindBest(size, alignment, padding, previousNode, foundNode);
				break;
			}
		}

		void FreeListAllocator::FindBest(const Size size, const Byte alignment, Size& padding, Node*& previousNode, Node*& foundNode)
		{
			// Iterate WHOLE list keeping a pointer to the best fit
			Size smallestDiff = (std::numeric_limits<Size>::max)();
			Node* bestBlock = nullptr;
			Node* it = m_freeList.Head,
				* itPrev = nullptr;
			while (it != nullptr)
			{
				padding = MemoryUtlis::CalculatePaddingWithHeader((Size)it, alignment, sizeof(FreeListAllocator::AllocHeader));
				const Size requiredSpace = size + padding;
				if (it->data.BlockSize >= requiredSpace && (it->data.BlockSize - requiredSpace < smallestDiff))
				{
					bestBlock = it;
				}

				smallestDiff = it->data.BlockSize;
				itPrev = it;
				it = it->next;
			}
			previousNode = itPrev;
			foundNode = bestBlock;
		}

		void
			FreeListAllocator::FindFirst(const Size size, const Byte alignment, Size& padding, Node*& previousNode, Node*& foundNode)
		{
			//Iterate list and return the first free block with a size >= than given size
			Node* it = m_freeList.Head, * itPrev = nullptr;

			while (it != nullptr)
			{
				padding = MemoryUtlis::CalculatePaddingWithHeader((Size)it, alignment, sizeof(FreeListAllocator::AllocHeader));
				const Size requiredSpace = size + padding;
				if (it->data.BlockSize >= requiredSpace)
				{
					break;
				}
				itPrev = it;
				it = it->next;
			}
			previousNode = itPrev;
			foundNode = it;
		}

		void FreeListAllocator::Reset()
		{
			Node* firstNode = (Node*)m_startPtr;
			firstNode->data.BlockSize = m_totalSize;
			firstNode->next = nullptr;
			m_freeList.Head = nullptr;
			m_freeList.Insert(nullptr, firstNode);
		}
	}