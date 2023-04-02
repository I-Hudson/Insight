#include "Core/Memory.h"

#ifdef IS_MEMORY_OVERRIDES
void* operator new(size_t size)
{
	if (size == 0)
	{
		return nullptr;
	}

	if (void* ptr = std::malloc(size))
	{
		if (Insight::Core::MemoryTracker::IsValidInstance())
		{
			Insight::Core::MemoryTracker::Instance().Track(ptr, size, Insight::Core::MemoryTrackAllocationType::Single);
		}
		return ptr;
	}
	return nullptr;
}

void* operator new[](size_t size)
{
	if (size == 0)
	{
		return nullptr;
	}

	if (void* ptr = std::malloc(size))
	{
		if (Insight::Core::MemoryTracker::IsValidInstance())
		{
			Insight::Core::MemoryTracker::Instance().Track(ptr, size, Insight::Core::MemoryTrackAllocationType::Array);
		}
		return ptr;
	}
	return nullptr;
}

void operator delete(void* ptr)
{
	if (Insight::Core::MemoryTracker::IsValidInstance())
	{
		Insight::Core::MemoryTracker::Instance().UnTrack(ptr);
	}
	std::free(ptr);
}

void operator delete[](void* ptr)
{
	if (Insight::Core::MemoryTracker::IsValidInstance())
	{
		Insight::Core::MemoryTracker::Instance().UnTrack(ptr);
	}
	std::free(ptr);
}
#endif

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
		TObjectOwnPtr<int> intPtr(new int{ 1 });

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

			derivedPtr.Reset();
			CHECK(!basePtr);
		}

		TEST_CASE("Owner in vector")
		{
			std::vector<TObjectOwnPtr<int>> OwnerPtrs;
			OwnerPtrs.push_back(TObjectOwnPtr<int>(new int{ 255 }));
			
			TObjectPtr objectPtr = OwnerPtrs.back();
			CHECK(objectPtr);

			OwnerPtrs.push_back(TObjectOwnPtr<int>(new int{ 512 }));

			CHECK(objectPtr);

			OwnerPtrs.erase(OwnerPtrs.begin());

			CHECK(!objectPtr);
		}
	}
}
#endif