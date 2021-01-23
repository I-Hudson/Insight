#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Module/Module.h"
#include "Engine/Input/InsightKeyCodes.h"
#include <functional>

struct GLFWwindow;
class Input;

namespace Module
{
	class WindowModule;

	class IS_API InputModule : public Module
	{
	public:
		InputModule(SharedPtr<WindowModule> windowModule);
		virtual ~InputModule() override;

		virtual void Update(const float& deltaTime) override;

	private:
		static void KeybordCallback(GLFWwindow* window, int key, int scanCode, int action, int mod);
		static void CharCallback(GLFWwindow* window, unsigned int c);
		static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mod);
		static void MousePositionCallback(GLFWwindow* window, double mouseX, double mouseY);

		void PollMousePosition();
		void PollMouseButton();

	private:
		struct KeyStates
		{
			unsigned int KeyDown : 1;
			unsigned int KeyUp : 1;
			unsigned int KeyPressed : 1;
			unsigned int KeyReleased : 1;
			unsigned int KeyHeld : 1;

			unsigned int MouseButtonDown : 1;
			unsigned int MouseButtonUp : 1;
			unsigned int MouseButtonPressed : 1;
			unsigned int MouseButtonReleased : 1;
		};

		static std::unordered_map<int, KeyStates> m_inputStates;
		static double m_mouseX;
		static double m_mouseY;

		SharedPtr<WindowModule> m_windowModule;

		friend Input;
	};
}