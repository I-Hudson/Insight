#include "ispch.h"
#include "Engine/Time/Time.h"

std::chrono::high_resolution_clock Time::s_clock;
std::chrono::steady_clock::time_point Time::s_startTimePoint;

RecordTimeType Time::s_recordTimeType = RecordTimeType::Milli;

double Time::s_timeScale = 1.0;
double Time::s_deltaTime = 0.0;
double Time::s_totalTime = 0.0;
double Time::s_averageFrameTime = 0.0;

int	   Time::s_previousFrameTimeIndex = 0;
double Time::s_previousFrameTimeSum = 0.0;
double Time::s_previousFrameTimes[s_PreviousFrameTimeCount];

void Time::UpdateTime()
{
	IS_PROFILE_FUNCTION();

	auto endTime = s_clock.now();
	if (s_startTimePoint.time_since_epoch().count() == 0)
	{
		s_startTimePoint = endTime;
	}
	s_deltaTime = std::chrono::duration<double, std::milli>(endTime - s_startTimePoint).count();
	s_totalTime += s_deltaTime;
	s_startTimePoint = s_clock.now();

	UpdateAverageFrameTime();
}

const float Time::GetDeltaTime()
{
	return static_cast<float>(ConvertToSec(s_deltaTime * s_timeScale));
}

const float Time::GetDeltaTimeRaw()
{
	return static_cast<float>(ConvertToSec(s_deltaTime));
}

const float Time::GetTotalTime()
{
	return static_cast<float>(ConvertToSec(s_totalTime));
}

const float Time::GetTimeScale()
{
	return static_cast<float>(s_timeScale);
}

void Time::SetTimeScale(const float& timeScale)
{
	s_timeScale = timeScale;
}

double Time::GetAverageFramerate()
{
	return s_averageFrameTime;
}

void Time::UpdateAverageFrameTime()
{
	s_previousFrameTimeSum -= s_previousFrameTimes[s_previousFrameTimeIndex];
	s_previousFrameTimes[s_previousFrameTimeIndex] = s_deltaTime;
	s_previousFrameTimeSum += s_previousFrameTimes[s_previousFrameTimeIndex];
	if (++s_previousFrameTimeIndex == s_PreviousFrameTimeCount)
	{
		s_previousFrameTimeIndex = 0;
	}

	s_averageFrameTime = s_previousFrameTimeSum / s_PreviousFrameTimeCount;
}

double Time::ConvertToMilli(const double& d)
{
	return d * 1000.0;
}
double Time::ConvertToSec(const double& d)
{
	return d / 1000.0;
}