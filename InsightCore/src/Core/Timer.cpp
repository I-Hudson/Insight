#include "Core/Timer.h"

namespace Insight
{
	namespace Core
	{
		void Timer::Start()
		{
			m_startTimePoint = std::chrono::high_resolution_clock::now();
		}

		void Timer::Stop()
		{
			m_stopTimePoint = std::chrono::high_resolution_clock::now();
		}

		void Timer::Continue()
		{
			m_stopTimePoint = { };
		}

		void Timer::Reset()
		{
			m_startTimePoint = std::chrono::high_resolution_clock::now();
			m_stopTimePoint = { };
		}

		DurationSec Timer::GetElapsedTime() const
		{
			return std::chrono::duration_cast<DurationSec>(GetElapsedTimeNano());
		}

		DurationNano Timer::GetElapsedTimeNano() const
		{
			return m_stopTimePoint.time_since_epoch() - m_startTimePoint.time_since_epoch();
		}

		DurationMill Timer::GetElapsedTimeMill() const
		{
			return std::chrono::duration_cast<DurationMill>(GetElapsedTimeNano());
		}
	}
}