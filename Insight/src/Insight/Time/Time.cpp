#include "ispch.h"
#include "Time.h"

namespace Insight
{
	std::chrono::high_resolution_clock Time::clock = std::chrono::high_resolution_clock();
	std::chrono::steady_clock::time_point Time::m_tpCurrentTime = std::chrono::steady_clock::time_point();

	double Time::m_prevTime = 0.0;
	double Time::m_timeScale = 1.0;
	double Time::m_currentTime = 0.0;
	double Time::m_deltaTime = 0.0;
	double Time::m_totalTime = 0.0;

	void Time::UpdateTime()
	{
		m_tpCurrentTime = clock.now();
		m_currentTime = std::chrono::duration<double>(m_tpCurrentTime.time_since_epoch()).count();
		m_deltaTime = m_currentTime - m_prevTime;
		m_prevTime = m_currentTime;
		m_totalTime += m_deltaTime / 100;
	}

	const float Time::GetDeltaTime()
	{
		return static_cast<float>(m_deltaTime * m_timeScale);
	}

	const float Time::GetDeltaTimeRaw()
	{
		return static_cast<float>(m_deltaTime);
	}

	const float Time::GetTotalTime()
	{
		return static_cast<float>(m_totalTime);
	}

	const float Time::GetTimeScale()
	{
		return static_cast<float>(m_timeScale);
	}

	void Time::SetTimeScale(const float& timeScale)
	{
		m_timeScale = timeScale;
	}
}