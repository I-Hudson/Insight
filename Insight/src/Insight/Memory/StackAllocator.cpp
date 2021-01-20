#include "ispch.h"
#include "Insight/Memory/StackAllocator.h"
#include "Insight/Memory/MemoryUtils.h"
#include "Insight/Core/Log.h"

	namespace Memory
	{
		StackAllocator::StackAllocator(Size stackSize)
			:m_top(0), m_totalSize(stackSize)
		{
			m_startPtr = malloc(stackSize);
			m_startAddress = reinterpret_cast<PtrInt>(m_startPtr);
		}

		StackAllocator::~StackAllocator()
		{
			if (m_top > 0)
			{
				IS_CORE_ASSERT(true, "StackAllocator: Memory leak.");
			}

			free(m_startPtr);
		}

		void* StackAllocator::Alloc(Size size, Byte alignment)
		{
			const Size currentAddress = (std::size_t)m_startPtr + m_top;

			std::size_t padding = MemoryUtlis::CalculatePaddingWithHeader(currentAddress, alignment, sizeof(AllocHeader));

			if (m_top + padding + size > m_totalSize) 
			{
				IS_CORE_ASSERT(true, "StackAllocator: => Not engough memory.");
			}
			m_top += padding;

			const Size nextAddress = currentAddress + padding;
			const Size headerAddress = nextAddress - sizeof(AllocHeader);
			AllocHeader allocationHeader{ static_cast<char>(padding) };
			AllocHeader* headerPtr = (AllocHeader*)headerAddress;
			headerPtr = &allocationHeader;

			m_top += size;

			IS_CORE_INFO("A \t@C {0} \t@R {1} \t0 {2} \tP {3}", (void*)currentAddress, (void*)nextAddress, m_top, padding);

			return (void*)nextAddress;
		}

		void StackAllocator::PrintUsed()
		{
			IS_CORE_INFO("Amount used: {0}. Total size: {1}. Perentage: {2}", m_top, m_totalSize, ((double)m_top / (double)m_totalSize) * 100);
		}

		void StackAllocator::FreeToMarker(const Marker marker)
		{
			// Move offset back to clear address
			const Size currentAddress = (Size) marker;
			const Size headerAddress = currentAddress - sizeof(AllocHeader);
			const AllocHeader* allocationHeader{ (AllocHeader*)headerAddress };

			m_top = currentAddress - allocationHeader->Padding - (Size) m_startPtr;

#ifdef IS_DEBUG
			IS_CORE_INFO("F\t@C {0}, \t@F {1} \tO {2} ", (void*)currentAddress, (void*)((char*)m_startPtr + m_top), m_top);
#endif
		}
	}