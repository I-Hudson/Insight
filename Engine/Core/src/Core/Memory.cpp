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
#define DOCTEST_CONFIG_IMPLEMENTATION_IN_DLL
#include "doctest.h"
namespace test
{
	struct Base
	{
		int I;
	};

	struct Derived : public Base
	{
		std::string Str;
	};

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

		TEST_CASE("Owner pointer to Object pointer")
		{
			TObjectOwnPtr<Derived> derivedPtr(new Derived);
			CHECK(derivedPtr);
			TObjectPtr<Base> basePtr = derivedPtr;
			CHECK(basePtr);
			CHECK(derivedPtr == basePtr);

			constexpr const char* TEST_STRING = "TestString";
			constexpr int TEST_INT = 45;
			derivedPtr->Str = TEST_STRING;
			derivedPtr->I = TEST_INT;

			CHECK(derivedPtr->Str == TEST_STRING);
			CHECK(basePtr->I == TEST_INT);
		}
	}
}
#endif