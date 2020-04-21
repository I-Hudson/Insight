#pragma once

#include "Insight/Core.h"

namespace Insight
{
	namespace Module
	{
		class WindowModule;
	}

	struct RendererStartUpData
	{
		Module::WindowModule* WindowModule;
	};

	enum class GraphicsAPI
	{
		None = 0,
		Vulkan = 1
	};

	class IS_API Renderer
	{
	public:
		virtual ~Renderer() { }

		virtual void Clear() = 0;
		virtual void Render() = 0;
		virtual void Present() = 0;

		static Renderer* Create(RendererStartUpData& startupData);
	};
}


