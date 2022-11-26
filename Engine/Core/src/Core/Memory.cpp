#include "Core/Memory.h"

namespace Insight::Core
{
	MemoryNewObject::MemoryNewObject(void* ptr)
	{
		Ptr = ptr;
		MemoryTracker::Instance().Track(ptr, MemoryTrackAllocationType::Single);
	}
}

#ifdef TEST_ENABLED
#include "doctest.h"
namespace test
{
	TEST_SUITE("TObjectOwnPtr")
	{
		int* rawInt = new int{ 8 };
		TObjectOwnPtr<int> intPtr(rawInt);
		TEST_CASE("Equal")
		{
			//TObjectOwnPtr<float> floatPtr = std::move(intPtr);
			CHECK(1 == 1);
		}
		//intPtr.Reset();
	}
}
#endif