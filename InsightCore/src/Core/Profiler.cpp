#include "Core/Profiler.h"
#include "Core/Timer.h"
#include "Core/Logger.h"


namespace Insight
{
	namespace Core
	{
		void WaitForProfiler()
		{
			const int timeoutSecs = 60;
			Timer timer;
			timer.Start();
#if defined(IS_PROFILE_ENABLED)

#if defined(IS_PROFILE_OPTICK)
			while (!Optick::IsActive())
#elif defined(IS_PROFILE_TRACY)
			while (!tracy::GetProfiler().IsConnected())
#endif
			{
#if defined(IS_PROFILE_OPTICK)
				Optick::Update();
#endif

				timer.Stop();
				DurationSec elapsedTime = timer.GetElapsedTime();
				if (elapsedTime.count() >= timeoutSecs)
				{
					IS_CORE_WARN("[WaitForProfiler] Timeout reached.");
					break;
				}
				timer.Continue();
				IS_CORE_INFO("[WaitForProfiler] Elapsed time seconds: {}", elapsedTime.count());
			}
#endif
		}
	}
}