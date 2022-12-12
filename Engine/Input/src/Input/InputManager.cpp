#include "Input/InputManager.h"

#include "Core/Logger.h"
#include "Core/ImGuiSystem.h"

#include "Graphics/Window.h"


#include <GLFW/glfw3.h>
#include <backends/imgui_impl_glfw.h>

namespace Insight
{
	namespace Input
	{
		bool InputManager::m_is_initialised = false;
		KeyMap InputManager::m_key_map;
		MouseState InputManager::m_mouse_state;

		std::vector<InputKeyCallbackFunc> InputManager::m_key_callback_funcs;
		std::vector<InputMousePosCallbackFunc> InputManager::m_mouse_pos_callback_funcs;
		std::vector<InputMouseButtonCallbackFunc> InputManager::m_mouse_button_callback_funcs;
		std::vector<InputMouseScrollCallbackFunc> InputManager::m_mouse_scroll_callback_funcs;

		void KeyState::Reset()
		{
			Pressed = Released == 1 ? 0 : Pressed;	/// If the was pressed and hasn't been released, then it is still pressed.
			Released = 0;							/// If we have released the key, then reset this. You can only release a key for one frame.
			Held = Released == 1 ? 0 : Held;		/// If the was held and hasn't been released, then it is still held.
		}

		bool InputManager::InitWithWindow(Graphics::Window* window)
		{
			if (window == nullptr)
			{
				IS_CORE_ERROR("[InputManager::InitWithWindow] 'Window' must be valid.");
				return false;
			}

			if (m_is_initialised)
			{
				IS_CORE_WARN("[InputManager::InitWithWindow] InputManager is already setup.");
				return false;
			}
			m_is_initialised = true;

			/// Set all out callback funcs, and call all our internal callbacks.

			glfwInit();

			glfwSetKeyCallback(window->GetRawWindow(), [](GLFWwindow* window, int key, int scancode, int action, int mods)
				{
					for (InputKeyCallbackFunc& func : m_key_callback_funcs) { func(key, scancode, action, mods); }
				});

			glfwSetCursorPosCallback(window->GetRawWindow(), [](GLFWwindow* window, double xpos, double ypos)
				{
					for (InputMousePosCallbackFunc& func : m_mouse_pos_callback_funcs) { func(static_cast<float>(xpos), static_cast<float>(ypos)); }
				});

			glfwSetMouseButtonCallback(window->GetRawWindow(), [](GLFWwindow* window, int button, int action, int mods)
				{
					for (InputMouseButtonCallbackFunc& func : m_mouse_button_callback_funcs) { func(button, action, mods); }
				});

			glfwSetScrollCallback(window->GetRawWindow(), [](GLFWwindow* window, double xoffset, double yoffset)
				{
					for (InputMouseScrollCallbackFunc& func : m_mouse_scroll_callback_funcs) { func(static_cast<float>(xoffset), static_cast<float>(yoffset)); }
				});

			RegisterKeyCallback([](int key, int scancode, int action, int mods)
				{
					m_key_map[key].Pressed	= action == GLFW_PRESS || action == GLFW_REPEAT;
					m_key_map[key].Released = action == GLFW_RELEASE;
					m_key_map[key].Held		= action == GLFW_REPEAT;
					m_key_map[key].Mod		= mods;
				});
			RegisterKeyCallback([](int key, int scancode, int action, int mods)
				{
					ImGui_ImplGlfw_KeyCallback(Graphics::Window::Instance().GetRawWindow(), key, scancode, action, mods);
				});

			RegisterMousePosCallback([](float xpos, float ypos)
				{
					m_mouse_state.X_Position = xpos;
					m_mouse_state.Y_Position = ypos;
				});
			RegisterMousePosCallback([](float xpos, float ypos)
				{
					ImGui_ImplGlfw_CursorPosCallback(Graphics::Window::Instance().GetRawWindow(), xpos, ypos);
				});

			RegisterMouseButtonCallback([](int button, int action, int mods)
				{
					m_mouse_state.Buttons[button].Pressed  = action == GLFW_PRESS || action == GLFW_REPEAT;
					m_mouse_state.Buttons[button].Released = action == GLFW_RELEASE;
					m_mouse_state.Buttons[button].Held     = action == GLFW_REPEAT;
					m_mouse_state.Buttons[button].Mod      = mods;

				});
			RegisterMouseButtonCallback([](int button, int action, int mods)
				{
					ImGui_ImplGlfw_MouseButtonCallback(Graphics::Window::Instance().GetRawWindow(), button, action, mods);
				});

			return true;
		}

		void InputManager::Update()
		{
			for (auto& pair : m_key_map)
			{
				///pair.second.Reset();
			}

			for (KeyState& button_state : m_mouse_state.Buttons)
			{
				///button_state.Reset();
			}
		}

		bool InputManager::IsKeyPressed(u32 key)
		{
			return m_key_map[key].Pressed;
		}

		bool InputManager::IsKeyReleased(u32 key)
		{
			return m_key_map[key].Released;
		}

		bool InputManager::IsKeyHeld(u32 key)
		{
			return m_key_map[key].Held;
		}

		bool InputManager::IsMouseButtonPressed(u32 key)
		{
			return m_mouse_state.Buttons.at(key).Pressed;
		}

		bool InputManager::IsMouseButtonReleased(u32 key)
		{
			return m_mouse_state.Buttons.at(key).Released;
		}

		bool InputManager::IsMouseButtonHeld(u32 key)
		{
			return m_mouse_state.Buttons.at(key).Held;
		}

		void InputManager::GetMousePosition(float& x_position, float& y_position)
		{
			x_position = m_mouse_state.X_Position;
			y_position = m_mouse_state.Y_Position;
		}
	}
}