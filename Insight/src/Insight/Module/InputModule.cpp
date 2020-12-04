#include "ispch.h"
#include "InputModule.h"
#include "Insight/Log.h"
#include "WindowModule.h"
#include "GLFW/glfw3.h"
#include "backends/imgui_impl_glfw.h"

namespace Insight
{
	namespace Module
	{
		std::unordered_map<int, Insight::Module::InputModule::KeyStates> InputModule::m_inputStates;
		double InputModule::m_mouseX;
		double InputModule::m_mouseY;

		InputModule::InputModule(WindowModule* windowModule)
		{
			m_windowModule = windowModule;

			glfwSetKeyCallback(m_windowModule->GetWindow()->m_window, KeybordCallback);
			glfwSetCharCallback(m_windowModule->GetWindow()->m_window, CharCallback);
			glfwSetMouseButtonCallback(m_windowModule->GetWindow()->m_window, MouseButtonCallback);
			glfwSetCursorPosCallback(m_windowModule->GetWindow()->m_window, MousePositionCallback);
		}

		InputModule::~InputModule()
		{

		}

		void InputModule::Update(const float& deltaTime)
		{ }

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

#ifdef IMGUI_ENABLED
			ImGui_ImplGlfw_KeyCallback(window, key, scanCode, action, mod);
#endif
		}

		void InputModule::CharCallback(GLFWwindow* window, unsigned int c)
		{
#ifdef IMGUI_ENABLED
			ImGui_ImplGlfw_CharCallback(window, c);
#endif
		}

		void InputModule::MousePositionCallback(GLFWwindow* window, double mouseX, double mouseY)
		{
			m_mouseX = mouseX;
			m_mouseY = mouseY;

#ifdef IMGUI_ENABLED
			ImGuiIO io = ImGui::GetIO();
			io.MousePos = ImVec2(m_mouseX, m_mouseY);
#endif
		}

		void InputModule::MouseButtonCallback(GLFWwindow* window, int button, int action, int mod)
		{
			switch (action)
			{
			case KEY_PRESS:
				m_inputStates[button].MouseButtonPressed = !m_inputStates[button].MouseButtonPressed;
				m_inputStates[button].MouseButtonReleased = !m_inputStates[button].MouseButtonPressed;
				break;
			case KEY_RELEASE:
				m_inputStates[button].MouseButtonReleased = !m_inputStates[button].MouseButtonReleased;
				m_inputStates[button].MouseButtonPressed = !m_inputStates[button].MouseButtonReleased;
				break;
			}

#ifdef IMGUI_ENABLED
			ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mod);
#endif
		}
	}
}