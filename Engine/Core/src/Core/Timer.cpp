#include "Core/Timer.h"
#include "Core/TypeAlias.h"

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
			m_elapsed_duration = m_stopTimePoint - m_startTimePoint;
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
			return m_elapsed_duration;
		}

		DurationMill Timer::GetElapsedTimeMill() const
		{
			return std::chrono::duration_cast<DurationMill>(GetElapsedTimeNano());
		}

		float Timer::GetElapsedTimeMillFloat() const
		{
			u32 raw_time = static_cast<u32>(GetElapsedTimeMill().count());
			float float_time = static_cast<float>(raw_time);
			return float_time / 1000.0f;
		}
	}
}