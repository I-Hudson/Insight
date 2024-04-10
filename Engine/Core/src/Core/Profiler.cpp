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
#else
			while(false)
#endif
			
			{
#if defined(IS_PROFILE_OPTICK)
				Optick::Update();
#endif

				timer.Stop();
				DurationSec elapsedTime = timer.GetElapsedTime();
				if (elapsedTime.count() >= timeoutSecs)
				{
					IS_LOG_CORE_WARN("[WaitForProfiler] Timeout reached.");
					break;
				}
				timer.Continue();
				IS_LOG_CORE_INFO("[WaitForProfiler] Elapsed time seconds: {}", elapsedTime.count());
			}
#endif
		}

		bool IsProfilerAttached()
		{
#if defined(IS_PROFILE_ENABLED)
#if defined(IS_PROFILE_OPTICK)	
			returnOptick::IsActive();
#elif defined(IS_PROFILE_TRACY) 
			return tracy::GetProfiler().IsConnected();
#else
			return false;
#endif
#endif
		}

		void ShutdownProfiler()
		{
#if defined(IS_PROFILE_ENABLED)
#if defined(IS_PROFILE_OPTICK)	
			
#elif defined(IS_PROFILE_TRACY) 
			if (IsProfilerAttached())
			{
				tracy::GetProfiler().RequestShutdown();
				while (!tracy::GetProfiler().HasShutdownFinished())
				{
				}
			}
#else
#endif
#endif
		}
	}
}