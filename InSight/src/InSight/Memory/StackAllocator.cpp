#include "ispch.h"
#include "Insight/Memory/StackAllocator.h"
#include "Insight/Memory/MemoryUtils.h"
#include "Insight/Log.h"

namespace Insight
{
	namespace Memory
	{
		StackAllocator::StackAllocator(Size stackSize)
			:m_top(0), m_totalSize(stackSize)
		{
			m_bottom = malloc(stackSize);
			m_bottomAddress = reinterpret_cast<PtrInt>(m_bottom);
		}

		StackAllocator::~StackAllocator()
		{
			free(m_bottom);
		}

		void* StackAllocator::Alloc(Size size, Byte alignment)
		{
			PtrInt rawAddress = m_bottomAddress + m_top;
			PtrInt misAllign = rawAddress & (alignment - 1);
			PtrDiff adjustment = alignment - misAllign;
			// For the special case when misAlignment = 0
			// make sure we don't shift the address by its alignment
			adjustment = adjustment & (alignment - 1);
			PtrInt alignedAddress = rawAddress + adjustment;
			Marker newTop = m_top + size + adjustment;

			if (newTop > m_totalSize)
			{
				IS_CORE_FATEL("StackAllocator: Alloc => Not engough memory.");
			}
			m_top = newTop;

			return reinterpret_cast<void*>(alignedAddress);
		}

		void StackAllocator::PrintUsed()
		{
			IS_CORE_INFO("Amount used: {0}. Total size: {1}. Perentage: {2}", m_top, m_totalSize, ((double)m_top / (double)m_totalSize) * 100);
		}

		void StackAllocator::FreeToMarker(const Marker marker)
		{
			PtrInt markerPtr = marker;
			markerPtr -= m_bottomAddress;

			m_top = markerPtr;
		}
	}
}

