#pragma once

#include "Defines.h"
#include <unordered_map>

namespace Insight
{
	namespace Core
	{
		enum class IS_CORE MemoryTrackAllocationType
		{
			Array,
			Single
		};

		struct IS_CORE MemoryTrackedAlloc
		{
			MemoryTrackedAlloc()
				: Ptr(nullptr), Type(MemoryTrackAllocationType::Array)
			{ }

			MemoryTrackedAlloc(void* ptr, MemoryTrackAllocationType type)
				: Ptr(ptr), Type(type)
			{ }

			void* Ptr;
			MemoryTrackAllocationType Type;
		};

		class IS_CORE MemoryTracker
		{
		public:

			static MemoryTracker& Instance()
			{
				static MemoryTracker instnace;
				return instnace;
			}
			void Destroy();

			void Track(void* ptr, MemoryTrackAllocationType type);
			void UnTrack(void* ptr);

		private:
			std::unordered_map<void*, MemoryTrackedAlloc> m_allocations;
		};
	}
}