
#include "Core/MemoryTracker.h"
#include "Core/Memory.h"

namespace Insight
{
    namespace Core
    {
		template<class _Ty>
		inline _CONSTEXPR20_DYNALLOC void STLNonTrackingAllocator<_Ty>::DeallocateInternal(u64 align, void* _Ptr, size_t _Bytes) noexcept
		{
			::DeleteNoTrack(_Ptr);
		}

		template<class _Ty>
		inline _CONSTEXPR20_DYNALLOC void* STLNonTrackingAllocator<_Ty>::AllocateInternal(u64 align, const size_t _Bytes)
		{
			if (_Bytes != 0)
			{
				return static_cast<_Ty*>(::NewNoTrack(_Bytes));
			}
			return nullptr;
		}
    }
}