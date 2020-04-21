#pragma once

#include "Insight/Core.h"
#include "Insight/Module/Module.h"
#include "Insight/Config/CVar.h"

#include "Insight/Renderer/Renderer.h"

namespace Insight
{
	namespace Module
	{
		class WindowModule;

		struct GraphicsModuleStartupData : public ModuleStartupData
		{
			WindowModule* WindowModule;
		};

		class IS_API GraphicsModule : public Module
		{
		public:
			GraphicsModule(ModuleStartupData& startupData = ModuleStartupData());
			virtual ~GraphicsModule() override;

			virtual void Update(const float& deltaTime) override;

			struct GraphicsConfig
			{
				CVar<int> GraphicsAPI{ "graphics_api", 1 };
			};

		private:
			WindowModule* m_windowModule;

			Renderer* m_renderer;
		};
	}
}

