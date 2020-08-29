#pragma once

#include "Insight/Core.h"
#include <chrono>

namespace Insight
{
	class IS_API Time
	{
	public:
		static void UpdateTime();
		static const float GetDeltaTime();
		static const float GetDeltaTimeRaw();
		static const float GetTotalTime();
		static const float GetTimeScale();
		static void SetTimeScale(const float& timeScale);

	private:
		static std::chrono::high_resolution_clock clock;
		static std::chrono::steady_clock::time_point m_tpCurrentTime;

		static double m_prevTime;
		static double m_timeScale;
		static double m_currentTime;
		static double m_deltaTime;
		static double m_totalTime;
	};
}