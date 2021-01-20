#pragma once

#include "Insight/Core/Core.h"
#include <chrono>

	enum RecordTimeType
	{
		Micro,
		Milli,
		Sec
	};

	class IS_API Time
	{
	public:
		static void UpdateTime();

		/* @breif This is in milli seconds */
		static const float GetDeltaTime();
		/* @breif This is in milli seconds */
		static const float GetDeltaTimeRaw();
		/* @breif This is in seconds */
		static const float GetTotalTime();
		static const float GetTimeScale();
		static void SetTimeScale(const float& timeScale);
		
		static double GetAverageFramerate();

	private:
		static void UpdateAverageFrameTime();

		static double ConvertFromRecordType(const double& d);
		static double ConvertToMilli(const double& d);
		static double ConvertToSec(const double& d);

	private:
		static std::chrono::high_resolution_clock s_clock;
		static std::chrono::steady_clock::time_point s_startTimePoint;

		static RecordTimeType s_recordTimeType;

		static double s_timeScale;
		static double s_deltaTime;
		static double s_totalTime;
		static double s_averageFrameTime;

		const static int s_PreviousFrameTimeCount = 120;
		static int s_previousFrameTimeIndex;
		static double s_previousFrameTimeSum;
		static double s_previousFrameTimes[s_PreviousFrameTimeCount];
	};