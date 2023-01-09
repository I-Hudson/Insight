#include "Graphics/Window.h"

#include "Core/Logger.h"
#include "Core/CommandLineArgs.h"

#include "Input/InputSystem.h"
#include "Input/InputStates/InputButtonState.h"
#include "Input/InputDevices/InputDevice_KeyboardMouse.h"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"

#include <stb_image.h>
#include <iostream>

namespace Insight
{
	namespace Graphics
	{
		constexpr const char* CMD_START_WINDOW_MINIMISED = "start_window_minimised";

		const std::unordered_map<u32, Input::MouseButtons> GLFW_MOUSE_BUTTONS_TO_INTERNAL =
		{
			{ GLFW_MOUSE_BUTTON_LEFT  , Input::MouseButtons::Left },
			{ GLFW_MOUSE_BUTTON_RIGHT , Input::MouseButtons::Right },
			{ GLFW_MOUSE_BUTTON_MIDDLE, Input::MouseButtons::Middle },
			{ GLFW_MOUSE_BUTTON_4     , Input::MouseButtons::Side0 },
			{ GLFW_MOUSE_BUTTON_5     , Input::MouseButtons::Side1 },
		};

		const std::unordered_map<u32, Input::KeyboardButtons> GLFW_KEYBOARD_BUTTONS_TO_INTERNAL =
		{
			{ GLFW_KEY_ESCAPE			, Input::KeyboardButtons::Key_Escape},

			{ GLFW_KEY_F1               , Input::KeyboardButtons::Key_F1 },
			{ GLFW_KEY_F2               , Input::KeyboardButtons::Key_F2 },
			{ GLFW_KEY_F3               , Input::KeyboardButtons::Key_F3 },
			{ GLFW_KEY_F4               , Input::KeyboardButtons::Key_F4 },
			{ GLFW_KEY_F5               , Input::KeyboardButtons::Key_F5 },
			{ GLFW_KEY_F6               , Input::KeyboardButtons::Key_F6 },
			{ GLFW_KEY_F7               , Input::KeyboardButtons::Key_F7 },
			{ GLFW_KEY_F8               , Input::KeyboardButtons::Key_F8 },
			{ GLFW_KEY_F9               , Input::KeyboardButtons::Key_F9 },
			{ GLFW_KEY_F10              , Input::KeyboardButtons::Key_F10 },
			{ GLFW_KEY_F11              , Input::KeyboardButtons::Key_F11 },
			{ GLFW_KEY_F12              , Input::KeyboardButtons::Key_F12 },

			{ GLFW_KEY_PRINT_SCREEN     , Input::KeyboardButtons::Key_PrintScreen },
			{ GLFW_KEY_SCROLL_LOCK      , Input::KeyboardButtons::Key_ScrollLock },
			{ GLFW_KEY_PAUSE            , Input::KeyboardButtons::Key_Pause },

			{ GLFW_KEY_GRAVE_ACCENT     , Input::KeyboardButtons::Key_Tilde },

			{ GLFW_KEY_0                , Input::KeyboardButtons::Key_0 },
			{ GLFW_KEY_1                , Input::KeyboardButtons::Key_1 },
			{ GLFW_KEY_2                , Input::KeyboardButtons::Key_2 },
			{ GLFW_KEY_3                , Input::KeyboardButtons::Key_3 },
			{ GLFW_KEY_4                , Input::KeyboardButtons::Key_4 },
			{ GLFW_KEY_5                , Input::KeyboardButtons::Key_5 },
			{ GLFW_KEY_6                , Input::KeyboardButtons::Key_6 },
			{ GLFW_KEY_7                , Input::KeyboardButtons::Key_7 },
			{ GLFW_KEY_8                , Input::KeyboardButtons::Key_8 },
			{ GLFW_KEY_9                , Input::KeyboardButtons::Key_9 },

			{ GLFW_KEY_MINUS            , Input::KeyboardButtons::Key_Minus },
			{ GLFW_KEY_EQUAL            , Input::KeyboardButtons::Key_Equals },
			{ GLFW_KEY_BACKSPACE        , Input::KeyboardButtons::Key_Backspace },

			{ GLFW_KEY_INSERT           , Input::KeyboardButtons::Key_Insert },
			{ GLFW_KEY_HOME             , Input::KeyboardButtons::Key_Home },
			{ GLFW_KEY_PAGE_UP          , Input::KeyboardButtons::Key_PageUp },
			{ GLFW_KEY_DELETE           , Input::KeyboardButtons::Key_Delete },
			{ GLFW_KEY_END              , Input::KeyboardButtons::Key_End },
			{ GLFW_KEY_PAGE_DOWN        , Input::KeyboardButtons::Key_PageDown},

			{ GLFW_KEY_UP				, Input::KeyboardButtons::Key_Up },
			{ GLFW_KEY_RIGHT			, Input::KeyboardButtons::Key_Right },
			{ GLFW_KEY_DOWN				, Input::KeyboardButtons::Key_Down },
			{ GLFW_KEY_LEFT				, Input::KeyboardButtons::Key_Left },

			{ GLFW_KEY_TAB				, Input::KeyboardButtons::Key_Tab },
			{ GLFW_KEY_CAPS_LOCK		, Input::KeyboardButtons::Key_CapsLock},

			{ GLFW_KEY_LEFT_SHIFT       , Input::KeyboardButtons::Key_LShift },
			{ GLFW_KEY_LEFT_CONTROL		, Input::KeyboardButtons::Key_LCtrl },
			{ GLFW_KEY_LEFT_SUPER		, Input::KeyboardButtons::Key_LSuper},
			{ GLFW_KEY_LEFT_ALT			, Input::KeyboardButtons::Key_LAlt },
			{ GLFW_KEY_SPACE            , Input::KeyboardButtons::Key_Space },
			{ GLFW_KEY_RIGHT_ALT		, Input::KeyboardButtons::Key_RAlt },
			{ GLFW_KEY_RIGHT_SUPER		, Input::KeyboardButtons::Key_RSuper},
			{ GLFW_KEY_MENU				, Input::KeyboardButtons::Key_Menu},
			{ GLFW_KEY_RIGHT_CONTROL	, Input::KeyboardButtons::Key_RCtrl},
			{ GLFW_KEY_RIGHT_SHIFT      , Input::KeyboardButtons::Key_RShift},
			{ GLFW_KEY_ENTER			, Input::KeyboardButtons::Key_Enter},

			{ GLFW_KEY_LEFT_BRACKET     , Input::KeyboardButtons::Key_LBracket },
			{ GLFW_KEY_RIGHT_BRACKET    , Input::KeyboardButtons::Key_LBracket },
			{ GLFW_KEY_SEMICOLON        , Input::KeyboardButtons::Key_SemiColon },
			{ GLFW_KEY_APOSTROPHE       , Input::KeyboardButtons::Key_Quote },
			{ GLFW_KEY_BACKSLASH		, Input::KeyboardButtons::Key_Comma },
			{ GLFW_KEY_COMMA            , Input::KeyboardButtons::Key_Comma },
			{ GLFW_KEY_PERIOD           , Input::KeyboardButtons::Key_Period },
			{ GLFW_KEY_SLASH            , Input::KeyboardButtons::Key_ForwardSlash },
			{ GLFW_KEY_WORLD_2			, Input::KeyboardButtons::Key_BackSlash },

			{ GLFW_KEY_A                , Input::KeyboardButtons::Key_A },
			{ GLFW_KEY_B                , Input::KeyboardButtons::Key_B },
			{ GLFW_KEY_C                , Input::KeyboardButtons::Key_C },
			{ GLFW_KEY_D                , Input::KeyboardButtons::Key_D },
			{ GLFW_KEY_E                , Input::KeyboardButtons::Key_E },
			{ GLFW_KEY_F                , Input::KeyboardButtons::Key_F },
			{ GLFW_KEY_G                , Input::KeyboardButtons::Key_G },
			{ GLFW_KEY_H                , Input::KeyboardButtons::Key_H },
			{ GLFW_KEY_I                , Input::KeyboardButtons::Key_I },
			{ GLFW_KEY_J                , Input::KeyboardButtons::Key_J },

			{ GLFW_KEY_K                , Input::KeyboardButtons::Key_K },
			{ GLFW_KEY_L                , Input::KeyboardButtons::Key_L },
			{ GLFW_KEY_M                , Input::KeyboardButtons::Key_M },
			{ GLFW_KEY_N                , Input::KeyboardButtons::Key_N },
			{ GLFW_KEY_O                , Input::KeyboardButtons::Key_O },
			{ GLFW_KEY_P                , Input::KeyboardButtons::Key_P },
			{ GLFW_KEY_Q                , Input::KeyboardButtons::Key_Q },
			{ GLFW_KEY_R                , Input::KeyboardButtons::Key_R },
			{ GLFW_KEY_S                , Input::KeyboardButtons::Key_S },
			{ GLFW_KEY_T                , Input::KeyboardButtons::Key_T },

			{ GLFW_KEY_U                , Input::KeyboardButtons::Key_U },
			{ GLFW_KEY_V                , Input::KeyboardButtons::Key_V },
			{ GLFW_KEY_W                , Input::KeyboardButtons::Key_W },
			{ GLFW_KEY_X                , Input::KeyboardButtons::Key_X },
			{ GLFW_KEY_Y                , Input::KeyboardButtons::Key_Y },
			{ GLFW_KEY_Z                , Input::KeyboardButtons::Key_Z },

			{ GLFW_KEY_NUM_LOCK         , Input::KeyboardButtons::Key_NumLock },
			{ GLFW_KEY_KP_DIVIDE        , Input::KeyboardButtons::Key_NumpadDivide },
			{ GLFW_KEY_KP_MULTIPLY      , Input::KeyboardButtons::Key_NumpadMultiply },
			{ GLFW_KEY_KP_SUBTRACT      , Input::KeyboardButtons::Key_NumpadMinus },
			{ GLFW_KEY_KP_ADD			, Input::KeyboardButtons::Key_NumpadPlus },
			{ GLFW_KEY_KP_ENTER			, Input::KeyboardButtons::Key_NumpadEnter },
			{ GLFW_KEY_KP_DECIMAL		, Input::KeyboardButtons::Key_NumpadPeriod },

			{ GLFW_KEY_KP_0				, Input::KeyboardButtons::Key_Numpad0 },
			{ GLFW_KEY_KP_1				, Input::KeyboardButtons::Key_Numpad1 },
			{ GLFW_KEY_KP_2				, Input::KeyboardButtons::Key_Numpad2 },
			{ GLFW_KEY_KP_3				, Input::KeyboardButtons::Key_Numpad3 },
			{ GLFW_KEY_KP_4				, Input::KeyboardButtons::Key_Numpad4 },
			{ GLFW_KEY_KP_5				, Input::KeyboardButtons::Key_Numpad5 },
			{ GLFW_KEY_KP_6				, Input::KeyboardButtons::Key_Numpad6 },
			{ GLFW_KEY_KP_7				, Input::KeyboardButtons::Key_Numpad7 },
			{ GLFW_KEY_KP_8				, Input::KeyboardButtons::Key_Numpad8 },
			{ GLFW_KEY_KP_9				, Input::KeyboardButtons::Key_Numpad9 },
		};

		std::unordered_map<GLFWwindow*, WindowInputs> Window::m_windowInputs;

		bool Window::Init(Input::InputSystem* inputSystem, int width, int height, std::string title)
		{
			return Init(inputSystem, false, width, height, std::move(title));
		}

		bool Window::Init(Input::InputSystem* inputSystem, bool startHidden, int width, int height, std::string title)
		{
			m_inputSystem = inputSystem;

			if (m_glfwInit)
			{
				IS_CORE_INFO("[Window::Init] Init already called.");
				return true;
			}

			m_title = std::move(title);
			m_size = { width, height };

			m_glfwInit = glfwInit();
			if (!m_glfwInit)
			{
				return false;
			}

			glfwSetErrorCallback([](int error_code, const char* description)
				{
					IS_CORE_ERROR("ErrorCode: {}\n Description: {}", error_code, description);
				});

			if (Core::CommandLineArgs::GetCommandLineValue(CMD_START_WINDOW_MINIMISED)->GetBool())
			{
				glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
			}

			if (startHidden)
			{
				glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
			}

			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
			m_glfwWindow = glfwCreateWindow(m_size.x, m_size.y, m_title.c_str(), NULL, NULL);

			int windowPosX, windowPosY;
			glfwGetWindowPos(m_glfwWindow, &windowPosX, &windowPosY);
			m_position = { windowPosX , windowPosY };

			glfwSetWindowPosCallback(m_glfwWindow, [](GLFWwindow* window, int xpos, int ypos)
				{
					Window::Instance().SetPosition({ xpos, ypos });
				});
			glfwSetWindowSizeCallback(m_glfwWindow, [](GLFWwindow* window, int width, int height)
				{
					Window::Instance().SetSize({ width, height });
				});

			m_windowInputs[m_glfwWindow] = WindowInputs{ m_glfwWindow };
			SetCallbacks();

			return true;
		}

		void Window::Destroy()
		{
			m_title.resize(0);

			if (m_glfwWindow)
			{
				glfwDestroyWindow(m_glfwWindow);
				m_glfwWindow = nullptr;
			}

			if (m_glfwInit)
			{
				ImGui_ImplGlfw_Shutdown();
				glfwTerminate();
				m_glfwInit = false;
			}
		}

		void Window::Update()
		{
			ASSERT(Platform::IsMainThread());
			for (auto& inputs: m_windowInputs)
			{
				inputs.second.Clear();
			}

			glfwPollEvents();

			for (auto& inputs : m_windowInputs)
			{
				m_inputSystem->UpdateInputs(inputs.second.Inputs);
			}
		}

		void Window::Rebuild()
		{
			std::string oldTitle = m_title;
			glm::ivec2 oldPosition = m_position;
			glm::ivec2 oldSize = m_size;

			Destroy();
			Init(m_inputSystem, oldSize.x, oldSize.y, oldTitle);
			SetPosition(oldPosition);
		}

		void Window::SetTite(std::string title)
		{
			m_title = std::move(title);
			glfwSetWindowTitle(m_glfwWindow, m_title.c_str());
		}

		void Window::SetIcon(const std::string& file_path)
		{
			int width, height, channels;
			void* pixels = stbi_load(file_path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
			if (!pixels)
			{
				return;
			}
			GLFWimage image;
			image.width = width;
			image.height = height;
			image.pixels = static_cast<unsigned char*>(pixels);
			glfwSetWindowIcon(m_glfwWindow, 1, &image);

			stbi_image_free(pixels);
		}

		void Window::SetX(int x)
		{

		}

		void Window::SetY(int y)
		{
		}

		void Window::SetPosition(glm::ivec2 position)
		{
			glfwSetWindowPos(m_glfwWindow, position.x, position.y);
			m_position = position;
		}

		void Window::SetWidth(int width)
		{
		}

		void Window::SetHeight(int height)
		{
		}

		void Window::SetSize(glm::ivec2 size)
		{
			m_size = size;
			glfwSetWindowSize(m_glfwWindow, m_size.x, m_size.y);
		}

		void Window::Show()
		{
			glfwShowWindow(m_glfwWindow);
		}

		void Window::Hide()
		{
			glfwHideWindow(m_glfwWindow);
		}

		bool Window::ShouldClose() const
		{
			if (!m_glfwInit || m_glfwWindow == nullptr)
			{
				return true;
			}
			return glfwWindowShouldClose(m_glfwWindow);
		}

		void Window::SetCallbacks()
		{
			glfwSetKeyCallback(m_glfwWindow, [](GLFWwindow* window, int key, int scancode, int action, int mods)
				{
					auto iter = GLFW_KEYBOARD_BUTTONS_TO_INTERNAL.find(static_cast<u32>(key));
					if (iter == GLFW_KEYBOARD_BUTTONS_TO_INTERNAL.end())
					{
						IS_CORE_ERROR("[Window.glfwSetKeyCallback] Unrecognised key '{}'.", key);
						return;
					}
					Input::KeyboardButtons keyboardButton = iter->second;

					WindowInputs& inputs = m_windowInputs[window];
					inputs.Inputs.push_back(
						Input::GenericInput
						{ 
							static_cast<u64>(0),
							Input::InputDeviceTypes::KeyboardMouse,
							Input::InputTypes::Button,
							static_cast<u64>(keyboardButton),
							static_cast<u64>(action == GLFW_RELEASE ? Input::ButtonStates::Released : action == GLFW_PRESS ?  Input::ButtonStates::Pressed : Input::ButtonStates::None),
							static_cast<u64>(mods)
						});
					ImGui_ImplGlfw_KeyCallback(Graphics::Window::Instance().GetRawWindow(), key, scancode, action, mods);
				});

			glfwSetCharCallback(m_glfwWindow, [](GLFWwindow* window, unsigned int codepoint)
				{
					ImGui_ImplGlfw_CharCallback(window, codepoint);
				});

			glfwSetMouseButtonCallback(m_glfwWindow, [](GLFWwindow* window, int button, int action, int mods)
				{
					auto iter = GLFW_MOUSE_BUTTONS_TO_INTERNAL.find(static_cast<u32>(button));
					ASSERT(iter != GLFW_MOUSE_BUTTONS_TO_INTERNAL.end());
					Input::MouseButtons mouseButton = iter->second;

					WindowInputs& inputs = m_windowInputs[window];
					inputs.Inputs.push_back(
					Input::GenericInput
					{
						static_cast<u64>(0),
						Input::InputDeviceTypes::KeyboardMouse,
						Input::InputTypes::MouseButton,
						static_cast<u64>(mouseButton),
						static_cast<u64>(action == GLFW_RELEASE ? Input::ButtonStates::Released : action == GLFW_PRESS ? Input::ButtonStates::Pressed : Input::ButtonStates::None),
						static_cast<u64>(mods)
					});
					ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
				});

			glfwSetCursorPosCallback(m_glfwWindow, [](GLFWwindow* window, double xpos, double ypos)
				{
					u64 xPosition;
					u64 yPosition;
					Platform::MemCopy(&xPosition, &xpos, sizeof(xPosition));
					Platform::MemCopy(&yPosition, &ypos, sizeof(yPosition));

					WindowInputs& inputs = m_windowInputs[window];
					inputs.Inputs.push_back(
					Input::GenericInput
					{
						static_cast<u64>(0),
						Input::InputDeviceTypes::KeyboardMouse,
						Input::InputTypes::Mouse,
						xPosition,
						yPosition,
						static_cast<u64>(0)
					});
					ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);
				});

			glfwSetScrollCallback(m_glfwWindow, [](GLFWwindow* window, double xoffset, double yoffset)
				{
					u64 xOffset;
					u64 yOffset;
					Platform::MemCopy(&xOffset, &xoffset, sizeof(xOffset));
					Platform::MemCopy(&yOffset, &yoffset, sizeof(yOffset));

					WindowInputs& inputs = m_windowInputs[window];
					inputs.Inputs.push_back(
						Input::GenericInput
						{
							static_cast<u64>(0),
							Input::InputDeviceTypes::KeyboardMouse,
							Input::InputTypes::MouseScroll,
							xOffset,
							yOffset,
							static_cast<u64>(0)
						});
					ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
				});
		}
	}
}