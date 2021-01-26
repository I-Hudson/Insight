#include "ispch.h"
#include "InputModule.h"
#include "Engine/Core/Log.h"
#include "ModuleManager.h"
#include "WindowModule.h"
#include "GLFW/glfw3.h"
#include "backends/imgui_impl_glfw.h"

namespace Module
{
	std::unordered_map<int, InputModule::KeyStates> InputModule::m_inputStates;
	double InputModule::m_mouseX;
	double InputModule::m_mouseY;

	InputModule::InputModule()
	{
		auto windowModule = ModuleManager::Instance()->GetModule<WindowModule>();
		glfwSetKeyCallback(windowModule->GetWindow()->m_window, KeybordCallback);
		glfwSetCharCallback(windowModule->GetWindow()->m_window, CharCallback);
		glfwSetMouseButtonCallback(windowModule->GetWindow()->m_window, MouseButtonCallback);
		glfwSetCursorPosCallback(windowModule->GetWindow()->m_window, MousePositionCallback);
	}

	InputModule::~InputModule()
	{

	}

	void InputModule::Update(const float& deltaTime)
	{
		for (auto& input : m_inputStates)
		{
			input.second.KeyPressed = 0;
			input.second.KeyReleased = 0;
			input.second.MouseButtonPressed = 0;
			input.second.MouseButtonReleased = 0;
		}
	}

	void InputModule::KeybordCallback(GLFWwindow* window, int key, int scanCode, int action, int mod)
	{
		switch (action)
		{
		case KEY_PRESS:
			m_inputStates[key].KeyDown = 1;
			m_inputStates[key].KeyUp = 0;
			m_inputStates[key].KeyPressed = 1;
			m_inputStates[key].KeyReleased = 0;
			break;
		case KEY_RELEASE:
			m_inputStates[key].KeyDown = 0;
			m_inputStates[key].KeyUp = 1;
			m_inputStates[key].KeyPressed = 0;
			m_inputStates[key].KeyReleased = 1;
			m_inputStates[key].KeyHeld = 0;
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
		io.MousePos = ImVec2(static_cast<float>(m_mouseX), static_cast<float>(m_mouseY));
#endif
	}

	void InputModule::MouseButtonCallback(GLFWwindow* window, int button, int action, int mod)
	{
		switch (action)
		{
		case KEY_PRESS:
			m_inputStates[button].MouseButtonDown = 1;
			m_inputStates[button].MouseButtonUp = 0;
			m_inputStates[button].MouseButtonPressed = 1;
			m_inputStates[button].MouseButtonReleased = 0;
			break;
		case KEY_RELEASE:
			m_inputStates[button].MouseButtonDown = 0;
			m_inputStates[button].MouseButtonUp = 1;
			m_inputStates[button].MouseButtonPressed = 0;
			m_inputStates[button].MouseButtonReleased = 1;
			break;
		}

#ifdef IMGUI_ENABLED
		ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mod);
#endif
	}
}