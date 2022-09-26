#include "Core/Memory.h"

namespace Insight::Core
{
	MemoryNewObject::MemoryNewObject(void* ptr)
	{
		Ptr = ptr;
		MemoryTracker::Instance().Track(ptr, MemoryTrackAllocationType::Single);
	}
}