#pragma once

#include "Insight/Core.h"
#include "Insight/Module/Module.h"
#include "Insight/Input/InsightKeyCodes.h"

struct GLFWwindow;
class Input;

namespace Insight
{

	namespace Module
	{
		class WindowModule;

		struct InputModuleData : public ModuleStartupData
		{
			WindowModule* WindowModule;
		};

		class IS_API InputModule : public Module
		{
		public:
			InputModule(ModuleStartupData& startupData);
			virtual ~InputModule() override;

			virtual void Update(const float& deltaTime) override;

		private:
			static void KeybordCallback(GLFWwindow* window, int key, int scanCode, int action, int mod);
			static void MouseCallback(GLFWwindow* window, int button, int action, int mod);

		private:
			static std::unordered_map<int, int> m_inputStates;

			WindowModule* m_windowModule;

			friend Input;
		};
	}
}
