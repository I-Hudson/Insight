#pragma once

#include "Core/Defines.h"
#include <chrono>

namespace Insight
{
	namespace Core
	{
		using DurationNano = std::chrono::nanoseconds;
		using DurationMill = std::chrono::milliseconds;
		using DurationSec = std::chrono::seconds;
		using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock, DurationNano>;

		class IS_CORE Timer
		{
		public:

			void Start();
			void Stop();
			void Continue();
			void Reset();

			DurationSec GetElapsedTime() const;
			DurationNano GetElapsedTimeNano() const;
			DurationMill GetElapsedTimeMill() const;

			float GetElapsedTimeMillFloat() const;

		private:
			/// Start time.
			TimePoint m_startTimePoint;
			/// End time.
			TimePoint m_stopTimePoint;
			DurationNano m_elapsed_duration;
		};
	}
}