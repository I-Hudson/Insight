#include "ispch.h"
#include "InputModule.h"

#include "WindowModule.h"
#include "GLFW/glfw3.h"

namespace Insight
{
	namespace Module
	{
		std::unordered_map<int, Insight::Module::InputModule::KeyStates> InputModule::m_inputStates;
		double InputModule::m_mouseX;
		double InputModule::m_mouseY;

		InputModule::InputModule(ModuleStartupData& startupData)
			: Module(startupData)
		{
			InputModuleData data = static_cast<InputModuleData&>(startupData);
			m_windowModule = data.WindowModule;

			glfwSetKeyCallback(m_windowModule->GetWindow()->m_window, KeybordCallback);
			glfwSetMouseButtonCallback(m_windowModule->GetWindow()->m_window, MouseButtonCallback);
			glfwSetCursorPosCallback(m_windowModule->GetWindow()->m_window, MousePositionCallback);
		}

		InputModule::~InputModule()
		{

		}

		void InputModule::Update(const float& deltaTime)
		{
		}

		void InputModule::KeybordCallback(GLFWwindow* window, int key, int scanCode, int action, int mod)
		{
			switch (action)
			{
			case KEY_PRESS:
				m_inputStates[key].KeyPressed = !m_inputStates[key].KeyPressed;
				m_inputStates[key].KeyReleased = !m_inputStates[key].KeyPressed;
				break;
			case KEY_RELEASE:
				m_inputStates[key].KeyReleased = !m_inputStates[key].KeyReleased;
				m_inputStates[key].KeyPressed = !m_inputStates[key].KeyReleased;
				m_inputStates[key].KeyHeld = !m_inputStates[key].KeyReleased;
				break;
			case KEY_REPEAT:
				m_inputStates[key].KeyHeld = !m_inputStates[key].KeyHeld;
				break;
			}
		}

		void InputModule::MousePositionCallback(GLFWwindow* window, double mouseX, double mouseY)
		{
			m_mouseX = mouseX;
			m_mouseY = mouseY;
		}

		void InputModule::MouseButtonCallback(GLFWwindow* window, int button, int action, int mod)
		{
			switch (action)
			{
			case KEY_PRESS:
				m_inputStates[button].MouseButtonPressed = !m_inputStates[button].MouseButtonPressed;
				break;
			case KEY_RELEASE:
				m_inputStates[button].MouseButtonReleased = !m_inputStates[button].MouseButtonReleased;
				break;
			}
		}
	}
}