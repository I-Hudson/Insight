#include "ispch.h"
#include "FreeListAllocator.h"
#include "Insight/Memory/MemoryUtils.h"

#include "Insight/Log.h"

#include <inttypes.h>

namespace Insight
{
	namespace Memory
	{
		FreeListAllocator::FreeListAllocator(const Size size, const PlacementPolicy policy)
			: m_sizeUsed(0), m_totalSize(size), m_policy(policy)
		{
			m_startPtr = malloc(size);
			m_startAddress = reinterpret_cast<PtrInt>(m_startPtr);
			Reset();
#if _DEBUG
			m_totalSize += size;
#endif
		}

		FreeListAllocator::~FreeListAllocator()
		{

#if _DEBUG
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

		void* FreeListAllocator::Alloc(const Size size, const Byte alignment)
		{
			const Size allocationHeaderSize = sizeof(FreeListAllocator::AllocHeader);
			//assert("Allocation size must be bigger" && size >= sizeof(Node));
			assert("Alignment must be 8 at least" && alignment >= 8);

			// Search through the free list for a free block that has enough space to allocate our data
			Size padding;
			Node* affectedNode, * previousNode;
			Find(size, alignment, padding, previousNode, affectedNode);
			IS_CORE_ASSERT(affectedNode != nullptr && "Not enough memory");

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
			allocHeader->AlignmentPadding = alignmentPadding;

			m_sizeUsed += requiredSize;

#ifdef _DEBUG
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
#if _DEBUG
			//sizeUsed -= allocHeader->size;
			if (m_monitorPureAlloc)
			{
				m_numOfFrees++;
			}
#endif
		}
		void FreeListAllocator::Coalescence(Node* previousNode, Node* freeNode)
		{
			if (freeNode->next != nullptr &&
				(Size)freeNode + freeNode->data.BlockSize == (Size)freeNode->next) 
			{
				freeNode->data.BlockSize += freeNode->next->data.BlockSize;
				m_freeList.Remove(freeNode, freeNode->next);
#ifdef _DEBUG
				IS_CORE_INFO("\tMerging(n) {0} % {1} \ts {2}", (void*)freeNode, (void*)freeNode->next, freeNode->data.BlockSize);
#endif
			}

			if (previousNode != nullptr &&
				(Size)previousNode + previousNode->data.BlockSize == (Size)freeNode) 
			{
				previousNode->data.BlockSize += freeNode->data.BlockSize;
				m_freeList.Remove(previousNode, freeNode);
#ifdef _DEBUG
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
}