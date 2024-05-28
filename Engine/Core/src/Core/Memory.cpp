#include "Core/Memory.h"
#include "Core/MemoryTracker.h"

namespace Insight::Memory
{
	void MemoryTrackPtr(void* ptr, const u64 size)
	{
		::Insight::Core::MemoryTracker::Instance().Track(ptr, size, Insight::Core::MemoryTrackAllocationType::Single);
	}

	void MemoryUnTrackPtr(void* ptr)
	{
		::Insight::Core::MemoryTracker::Instance().UnTrack(ptr);
	}
}

void* NewBytes(u64 bytes, Insight::Core::MemoryAllocCategory memoryAllocCategory)
{
	void* ptr = std::malloc(bytes);
	Insight::Core::MemoryTracker::Instance().Track(ptr, bytes, memoryAllocCategory, Insight::Core::MemoryTrackAllocationType::Array);
	return ptr;
}

void* NewBytes(u64 bytes)
{
	return NewBytes(bytes, Insight::Core::MemoryAllocCategory::General);
}

#ifdef TEST_ENABLED
//#define DOCTEST_CONFIG_IMPLEMENTATION_IN_DLL
//#include "doctest.h"
//namespace test
//{
//	struct Base
//	{
//		int I;
//	};
//
//	struct Derived : public Base
//	{
//		std::string Str;
//	};
//
//	TEST_SUITE("TObjectOwnPtr")
//	{
//		TObjectOwnPtr<int> intPtr(new int{ 1 });
//
//		TEST_CASE("Owner pointer to Object pointer")
//		{
//			TObjectOwnPtr<Derived> derivedPtr(new Derived);
//			CHECK(derivedPtr);
//			TObjectPtr<Base> basePtr = derivedPtr;
//			CHECK(basePtr);
//			CHECK(derivedPtr == basePtr);
//
//			constexpr const char* TEST_STRING = "TestString";
//			constexpr int TEST_INT = 45;
//			derivedPtr->Str = TEST_STRING;
//			derivedPtr->I = TEST_INT;
//
//			CHECK(derivedPtr->Str == TEST_STRING);
//			CHECK(basePtr->I == TEST_INT);
//
//			derivedPtr.Reset();
//			CHECK(!basePtr);
//		}
//
//		TEST_CASE("Owner in vector")
//		{
//			std::vector<TObjectOwnPtr<int>> OwnerPtrs;
//			OwnerPtrs.push_back(TObjectOwnPtr<int>(new int{ 255 }));
//			
//			TObjectPtr objectPtr = OwnerPtrs.back();
//			CHECK(objectPtr);
//
//			OwnerPtrs.push_back(TObjectOwnPtr<int>(new int{ 512 }));
//
//			CHECK(objectPtr);
//
//			OwnerPtrs.erase(OwnerPtrs.begin());
//
//			CHECK(!objectPtr);
//		}
//	}
//}
#endif