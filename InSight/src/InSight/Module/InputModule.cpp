#include "ispch.h"
#include "InputModule.h"

#include "WindowModule.h"
#include "GLFW/glfw3.h"

namespace Insight
{
	namespace Module
	{
		std::unordered_map<int, int> InputModule::m_inputStates;
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
			m_inputStates.clear();
		}

		void InputModule::KeybordCallback(GLFWwindow* window, int key, int scanCode, int action, int mod)
		{
			m_inputStates[key] = action;
		}

		void InputModule::MousePositionCallback(GLFWwindow* window, double mouseX, double mouseY)
		{
			m_mouseX = mouseX;
			m_mouseY = mouseY;
		}

		void InputModule::MouseButtonCallback(GLFWwindow* window, int button, int action, int mod)
		{
			m_inputStates[button] = action;
		}
	}
}