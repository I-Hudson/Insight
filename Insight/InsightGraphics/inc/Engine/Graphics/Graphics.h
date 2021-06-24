#pragma once

#include "Engine/Templates/TSingleton.h"

namespace Insight::Graphics
{
	class GraphicsInfo : public ::Insight::Core::TSingleton<GraphicsInfo>
	{
	public:
		virtual bool IsD311() = 0;
		virtual bool IsD312() = 0;
		virtual bool IsVulkan() = 0;
		virtual bool IsOpenGL() = 0;
	};
}

/// <summary>
/// Util class for accessing in engine data in an easy way.
/// </summary>
class Graphics
{
public:

	static bool IsD311() { return Insight::Graphics::GraphicsInfo::Instance()->IsD311(); }
	static bool IsD312() { return Insight::Graphics::GraphicsInfo::Instance()->IsD312(); }
	static bool IsVulkan() { return Insight::Graphics::GraphicsInfo::Instance()->IsVulkan(); }
	static bool IsOpenGL() { return Insight::Graphics::GraphicsInfo::Instance()->IsOpenGL(); }
};