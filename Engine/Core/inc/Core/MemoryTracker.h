#pragma once

#include "Defines.h"
#include "Core/Singleton.h"

#include <unordered_map>
#include <unordered_set>
#include <array>
#include <mutex>

namespace Insight
{
	namespace Core
	{
		CONSTEXPR int c_CallStackCount = 32;

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

		class IS_CORE MemoryTracker : public Core::Singleton<MemoryTracker>
		{
		public:
			~MemoryTracker();

			void Initialise();
			void Destroy();

			void Track(void* ptr, u64 size, MemoryTrackAllocationType type);
			void UnTrack(void* ptr);
			void NameAllocation(void* ptr, const char* name);

		private:
			std::array < std::string, c_CallStackCount> GetCallStack();

		private:
#ifdef IS_MEMORY_TRACKING
			std::unordered_map<void*, MemoryTrackedAlloc> m_allocations;
			bool m_symInitialize = false;
			bool m_isReady = false;
			std::mutex m_lock;
#endif // IS_MEMORY_TRACKING
		};
	}
}