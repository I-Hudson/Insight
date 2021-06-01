#pragma once

#include "JobSystemManager.h"
#include "Engine/Templates/TSingleton.h"
//Simple alias of the JobSystem sub module to keep it inline with Insight name spaces.

namespace Insight::JS
{
	class JobSystemManager : public Core::TSingleton<JobSystemManager>, public ::js::JobSystemManager
	{ };
}