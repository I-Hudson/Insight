#pragma once

#include "Insight/Core.h"
#include "Insight/Module/Module.h"

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
			GraphicsModule();
			virtual ~GraphicsModule() override;

			virtual void Startup(ModuleStartupData startupData = ModuleStartupData()) override;
			virtual void Shutdown() override;
			virtual void Update(const float& deltaTime) override;

		private:
			WindowModule* m_windowModule;
		};
	}
}

