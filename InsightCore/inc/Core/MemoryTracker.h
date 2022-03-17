#pragma once

#include "Defines.h"
#include <unordered_map>
#include <array>

namespace Insight
{
	namespace Core
	{
		constexpr int c_CallStackCount = 32;

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
			MemoryTrackedAlloc(void* ptr, MemoryTrackAllocationType type, std::array<std::string, c_CallStackCount> callStack)
				: Ptr(ptr), Type(type), CallStack(std::move(callStack))
			{ }

			void* Ptr;
			MemoryTrackAllocationType Type;
			std::array<std::string, c_CallStackCount> CallStack;
		};

		class IS_CORE MemoryTracker
		{
		public:
			~MemoryTracker();

			static MemoryTracker& Instance()
			{
				static MemoryTracker instnace;
				return instnace;
			}
			void Destroy();

			void Track(void* ptr, MemoryTrackAllocationType type);
			void UnTrack(void* ptr);

		private:
			std::array < std::string, c_CallStackCount> GetCallStack();

		private:
			std::unordered_map<void*, MemoryTrackedAlloc> m_allocations;
			bool m_symInitialize = false;
		};
	}
}