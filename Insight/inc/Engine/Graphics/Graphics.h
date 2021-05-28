#pragma once

#include "Engine/Module/GraphicsModule.h"

/// <summary>
/// Util class for accessing in engine data in an easy way.
/// </summary>
class Graphics
{
public:

	static bool IsD311() { return Insight::Module::GraphicsModule::Instance()->IsD311(); }
	static bool IsD312() { return Insight::Module::GraphicsModule::Instance()->IsD312(); }
	static bool IsVulkan() { return Insight::Module::GraphicsModule::Instance()->IsVulkan(); }
	static bool IsOpenGL() { return Insight::Module::GraphicsModule::Instance()->IsOpenGL(); }
};