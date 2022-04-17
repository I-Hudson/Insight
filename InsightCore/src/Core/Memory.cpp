#include "Core/Memory.h"

namespace Insight::Core
{
	MemoryNewObject::MemoryNewObject(void* ptr)
	{
		Ptr = ptr;
		Insight::Core::MemoryTracker::Instance().Track(ptr, Insight::Core::MemoryTrackAllocationType::Single);
	}
}