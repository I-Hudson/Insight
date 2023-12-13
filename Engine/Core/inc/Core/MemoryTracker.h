#pragma once

#include "Defines.h"
#include "Core/Singleton.h"

#include <unordered_map>
#include <unordered_set>
#include <array>
#include <mutex>

//#undef IS_MEMORY_TRACKING

namespace Insight
{
	namespace Core
	{
#ifdef IS_MEMORY_TRACKING
		CONSTEXPR int c_CallStackCount = 32;
		CONSTEXPR u64 c_CallstackStringSize = 512;
#else
		CONSTEXPR int c_CallStackCount = 1;
		CONSTEXPR u64 c_CallstackStringSize = 1;
#endif

		enum class IS_CORE MemoryTrackAllocationType
		{
			Array,
			Single
		};

		enum class MemoryAllocCategory
		{
			General,
			Threading,
			Maths,
			Input,
			Graphics,
			Resources,
			ECS,
			World,
			Serialiser,
			Editor,

			Size
		};
		constexpr const char* MemoryAllocCategoryToString[] =
		{
			"General",
			"Threading",
			"Maths",
			"Input",
			"Graphics",
			"Resources",
			"ECS",
			"World",
			"Serialiser",
			"Editor",
		};
		static_assert(ARRAY_COUNT(MemoryAllocCategoryToString) == static_cast<u64>(MemoryAllocCategory::Size));

		/// @brief Copy of STL allocator but doesn't track memory allocations.
		/// Currently used for the internal map within MemoryTracker as if the map tracks it's own allocations
		/// then we get a circle dependency.
		/// @tparam _Ty 
		template <class _Ty>
		struct STLNonTrackingAllocator
		{
		public:
			static_assert(!std::is_const_v<_Ty>, "The C++ Standard forbids containers of const elements "
				"because allocator<const T> is ill-formed.");

			using _From_primary = STLNonTrackingAllocator;

			using value_type = _Ty;

#if _HAS_DEPRECATED_ALLOCATOR_MEMBERS
			_CXX17_DEPRECATE_OLD_ALLOCATOR_MEMBERS typedef _Ty* pointer;
			_CXX17_DEPRECATE_OLD_ALLOCATOR_MEMBERS typedef const _Ty* const_pointer;

			_CXX17_DEPRECATE_OLD_ALLOCATOR_MEMBERS typedef _Ty& reference;
			_CXX17_DEPRECATE_OLD_ALLOCATOR_MEMBERS typedef const _Ty& const_reference;
#endif // _HAS_DEPRECATED_ALLOCATOR_MEMBERS

			using size_type = size_t;
			using difference_type = ptrdiff_t;

			using propagate_on_container_move_assignment = std::true_type;

#if _HAS_DEPRECATED_ALLOCATOR_MEMBERS
			using is_always_equal _CXX17_DEPRECATE_OLD_ALLOCATOR_MEMBERS = std::true_type;

			template <class _Other>
			struct _CXX17_DEPRECATE_OLD_ALLOCATOR_MEMBERS rebind {
				using other = STLNonTrackingAllocator<_Other>;
			};

			_CXX17_DEPRECATE_OLD_ALLOCATOR_MEMBERS _NODISCARD _Ty* address(_Ty& _Val) const noexcept {
				return _STD addressof(_Val);
			}

			_CXX17_DEPRECATE_OLD_ALLOCATOR_MEMBERS _NODISCARD const _Ty* address(const _Ty& _Val) const noexcept {
				return _STD addressof(_Val);
			}
#endif // _HAS_DEPRECATED_ALLOCATOR_MEMBERS

			constexpr STLNonTrackingAllocator() noexcept {}

			constexpr STLNonTrackingAllocator(const STLNonTrackingAllocator&) noexcept = default;
			template <class _Other>
			constexpr STLNonTrackingAllocator(const STLNonTrackingAllocator<_Other>&) noexcept {}
			~STLNonTrackingAllocator() = default;
			STLNonTrackingAllocator& operator=(const STLNonTrackingAllocator&) = default;

			void deallocate(_Ty* const _Ptr, const size_t _Count)
			{
				// no overflow check on the following multiply; we assume _Allocate did that check
				destroy(_Ptr);
				DeallocateInternal(std::_New_alignof<_Ty>, _Ptr, sizeof(_Ty) * _Count);
			}

			_NODISCARD __declspec(allocator) _Ty* allocate(_CRT_GUARDOVERFLOW const size_t _Count)
			{
				return static_cast<_Ty*>(AllocateInternal(std::_New_alignof<_Ty>, std::_Get_size_of_n<sizeof(_Ty)>(_Count)));
			}

#if _HAS_DEPRECATED_ALLOCATOR_MEMBERS
			_CXX17_DEPRECATE_OLD_ALLOCATOR_MEMBERS _NODISCARD __declspec(allocator) _Ty* allocate(
				_CRT_GUARDOVERFLOW const size_t _Count, const void*) {
				return allocate(_Count);
			}

			_CXX17_DEPRECATE_OLD_ALLOCATOR_MEMBERS _NODISCARD size_t max_size() const noexcept {
				return static_cast<size_t>(-1) / sizeof(_Ty);
			}
#endif // _HAS_DEPRECATED_ALLOCATOR_MEMBERS

		private:
			void destroy(_Ty* const _Ptr) 
			{
				_Ptr->~_Ty();
			}
			template <class _Objty, class... _Types>
			_CXX17_DEPRECATE_OLD_ALLOCATOR_MEMBERS void construct(_Objty* const _Ptr, _Types&&... _Args) 
			{
				::new (std::_Voidify_iter(_Ptr)) _Objty(_STD forward<_Types>(_Args)...);
			}

			constexpr __declspec(allocator) void DeallocateInternal(u64 align, void* _Ptr, size_t _Bytes) noexcept;

			constexpr __declspec(allocator) void* AllocateInternal(u64 align, const size_t _Bytes);
		};

		struct IS_CORE MemoryTrackedAlloc
		{
			MemoryTrackedAlloc()
				: Ptr(nullptr), Size(0), Category(MemoryAllocCategory::General), Type(MemoryTrackAllocationType::Array)
			{ }
			MemoryTrackedAlloc(void* ptr, u64 size, MemoryAllocCategory category, MemoryTrackAllocationType type)
				: Ptr(ptr), Size(size), Category(category), Type(type)
			{ }
			MemoryTrackedAlloc(void* ptr, u64 size, MemoryAllocCategory category, MemoryTrackAllocationType type, std::array<char[c_CallstackStringSize], c_CallStackCount> callStack)
				: Ptr(ptr), Size(size), Category(category), Type(type), CallStack(std::move(callStack))
			{ }

			void* Ptr;
			u64 Size;
			MemoryTrackAllocationType Type;
			MemoryAllocCategory Category;
			std::array<char[c_CallstackStringSize], c_CallStackCount> CallStack;
		};

		class IS_CORE MemoryTracker
		{
			THREAD_SAFE;
		public:
			~MemoryTracker();

			static MemoryTracker& Instance()
			{
				static MemoryTracker instance;
				return instance;
			}

			void Initialise();
			void Destroy();

			void Track(void* ptr, u64 size, MemoryTrackAllocationType type);
			void Track(void* ptr, u64 size, MemoryAllocCategory category, MemoryTrackAllocationType type);

			void UnTrack(void* ptr);

			void NameAllocation(void* ptr, const char* name);

			/// @brief Get the memory usage for a category in bytes.
			/// @param category 
			/// @return u64
			u64 GetUsage(MemoryAllocCategory category) const;
			/// @brief Get the total number of allocations for a single category.
			/// @param category 
			/// @return u64
			u64 GetTotalNumberOfAllocationsForCategory(MemoryAllocCategory category) const;
			/// @brief Get the total number of allocations being tracked.
			/// @return u64
			u64 GetTotalNumberOfAllocations() const;
			/// @brief Get the total amount of allocated bytes being tracked.
			/// @return u64
			u64 GetTotalAllocatedInBytes() const;

		private:
			std::array<char[c_CallstackStringSize], c_CallStackCount> GetCallStack();

		private:
			std::unordered_map<void*, MemoryTrackedAlloc, std::hash<void*>, std::equal_to<void*>, STLNonTrackingAllocator<std::pair<void* const, MemoryTrackedAlloc>>> m_allocations;
			std::unordered_map<void*, std::string> m_allocationToName;
			mutable std::mutex m_allocationToNameLock;

			bool m_symInitialize = false;
			bool m_isReady = false;

			mutable std::mutex m_lock;
			u64 m_totalAllocatedInBytes = 0;
			std::array<u64, static_cast<u64>(MemoryAllocCategory::Size)> m_categoryAllocationSizeBytes;
			std::array<u64, static_cast<u64>(MemoryAllocCategory::Size)> m_categoryAllocationCount;
		};
	}
}

#include "Core/MemoryTracker.inl"