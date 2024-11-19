#include "CoreModule.h"

#include "Core/Logger.h"

namespace Insight
{
	void CoreModule::Initialise()
	{
#ifndef IS_MONOLITH
		SET_SPDLOG_LOGGERS();
#endif
	}
}