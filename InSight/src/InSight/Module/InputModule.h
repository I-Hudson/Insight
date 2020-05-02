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
			static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mod);
			static void MousePositionCallback(GLFWwindow* window, double mouseX, double mouseY);

		private:
			struct KeyStates
			{
				unsigned int KeyPressed : 1;
				unsigned int KeyReleased : 1;
				unsigned int KeyHeld : 1;
				unsigned int MouseButtonPressed : 1;
				unsigned int MouseButtonReleased : 1;
			};

			static std::unordered_map<int, KeyStates> m_inputStates;
			static double m_mouseX;
			static double m_mouseY;

			WindowModule* m_windowModule;

			friend Input;
		};
	}
}
