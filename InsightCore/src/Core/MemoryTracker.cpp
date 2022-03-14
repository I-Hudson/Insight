#include "Core/MemoryTracker.h"
#include <iostream>

namespace Insight
{
	namespace Core
	{
		void MemoryTracker::Destroy()
		{
			if (m_allocations.size() > 0)
			{
				std::cout << "memeory leak\n";
			}
		}

		void MemoryTracker::Track(void* ptr, MemoryTrackAllocationType type)
		{
			auto itr = m_allocations.find(ptr);
			if (itr != m_allocations.end())
			{
				// Allocation already tracked.
				return;
			}
			m_allocations[ptr] = MemoryTrackedAlloc(ptr, type);
		}

		void MemoryTracker::UnTrack(void* ptr)
		{
			auto itr = m_allocations.find(ptr);
			if (itr != m_allocations.end())
			{
				m_allocations.erase(itr);
			}
		}
	}
}