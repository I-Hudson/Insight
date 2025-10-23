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

#if IS_TESTING
#define DOCTEST_CONFIG_IMPLEMENTATION_IN_DLL
#include "doctest.h"
namespace test
{
	using namespace Insight;

	TEST_SUITE("Memory")
	{
		TEST_CASE("New")
		{
			const u64 startAllocationCount = Insight::Core::MemoryTracker::Instance().GetTotalNumberOfAllocations();
			const u64 startAllocationByteSize = Insight::Core::MemoryTracker::Instance().GetTotalAllocatedInBytes();
			int* intPtr = ::New<int>(45);

			CHECK(*intPtr == 45);
			CHECK(Insight::Core::MemoryTracker::Instance().GetTotalNumberOfAllocations() == startAllocationCount + 1);
			CHECK(Insight::Core::MemoryTracker::Instance().GetTotalAllocatedInBytes() == startAllocationByteSize + sizeof(int));

			::Delete(intPtr);

			CHECK(Insight::Core::MemoryTracker::Instance().GetTotalNumberOfAllocations() == startAllocationCount);
			CHECK(Insight::Core::MemoryTracker::Instance().GetTotalAllocatedInBytes() == startAllocationByteSize);
		}
	}

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
		TObjectOwnPtr<int> intPtr(::New<int>(1));

		TEST_CASE("Owner pointer to Object pointer")
		{
			TObjectOwnPtr<Derived> derivedPtr(::New<Derived>());
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
			OwnerPtrs.push_back(TObjectOwnPtr<int>(::New<int>(255)));

			TObjectPtr objectPtr = OwnerPtrs.back();
			CHECK(objectPtr);

			OwnerPtrs.push_back(TObjectOwnPtr<int>(::New<int>(512)));

			CHECK(objectPtr);

			OwnerPtrs.erase(OwnerPtrs.begin());

			CHECK(!objectPtr);
		}
	}

	TEST_SUITE("UPtr")
	{
		TEST_CASE("Creation")
		{
			{
				UPtr<int> nullPtr;
				CHECK_FALSE(nullPtr);
				CHECK(nullPtr == nullptr);
				CHECK_FALSE(nullPtr.IsValid());
			}

			{
				UPtr<int> intPtr(::New<int>(45));
				CHECK(intPtr);
				CHECK(intPtr != nullptr);
				CHECK(intPtr.IsValid());
			}

			{
				UPtr<int> intPtrBase(::New<int>(45));
				CHECK(intPtrBase);
				CHECK(intPtrBase != nullptr);
				CHECK(intPtrBase.IsValid());

				UPtr<int> intPtr(std::move(intPtrBase));
				CHECK(intPtr);
				CHECK(intPtr != nullptr);
				CHECK(intPtr.IsValid());

				CHECK_FALSE(intPtrBase);
				CHECK(intPtrBase == nullptr);
				CHECK_FALSE(intPtrBase.IsValid());
			}
		}
	}

	TEST_SUITE("RPtr")
	{
		TEST_CASE("New")
		{
			{
				RPtr<int> nullPtr;
				CHECK_FALSE(nullPtr);
				CHECK(nullPtr == nullptr);
				CHECK_FALSE(nullPtr.IsValid());
			}

			{
				RPtr<int> intPtr(::New<int>(45));
				CHECK(intPtr);
				CHECK(intPtr != nullptr);
				CHECK(intPtr.IsValid());
			}
		}

		TEST_CASE("Copy")
		{
			{
				RPtr<int> intPtrBase(::New<int>(45));
				CHECK(intPtrBase);
				CHECK(intPtrBase != nullptr);
				CHECK(intPtrBase.IsValid());

				RPtr<int> intPtr(intPtrBase);
				CHECK(intPtr);
				CHECK(intPtr != nullptr);
				CHECK(intPtr.IsValid());

				CHECK(intPtrBase);
				CHECK(intPtrBase != nullptr);
				CHECK(intPtrBase.IsValid());

				CHECK(intPtrBase == intPtr);

				intPtrBase.Reset();
				CHECK(intPtr);
				intPtr.Reset();

				CHECK(!intPtrBase);
				CHECK(!intPtr);
			}
		}

		TEST_CASE("Move")
		{
			{
				RPtr<int> intPtrBase(::New<int>(45));
				CHECK(intPtrBase);
				CHECK(intPtrBase != nullptr);
				CHECK(intPtrBase.IsValid());

				RPtr<int> intPtr(std::move(intPtrBase));
				CHECK(intPtr);
				CHECK(intPtr != nullptr);
				CHECK(intPtr.IsValid());

				CHECK_FALSE(intPtrBase);
				CHECK(intPtrBase == nullptr);
				CHECK_FALSE(intPtrBase.IsValid());

				intPtr.Reset();
				CHECK(!intPtrBase);
				CHECK(!intPtr);
			}
		}
	}

	TEST_SUITE("WPtr")
	{

	}
}
#endif // IS_TESTING