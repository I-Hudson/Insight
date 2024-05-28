#include "CoreModule.h"

#include "Core/Logger.h"

namespace Insight
{
	void CoreModule::Initialise()
	{
		SET_SPDLOG_LOGGERS();
	}
}