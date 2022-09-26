#pragma once

#include "Input/Defines.h"
#include "Input/KeyList.h"

#include "Core/TypeAlias.h"

#include <unordered_map>
#include <functional>
#include <array>

namespace Insight
{
	namespace Graphics
	{
		class Window;
	}

	namespace Input
	{
		//// <summary>
		//// State of a single key.
		//// </summary>
		struct KeyState
		{
			void Reset();

			u8 Pressed	: 1;		/// Has a key been pressed this frame. This is reset at the end of the frame.
			u8 Released : 1;		/// Has a key been released this frame. This is reset at the end of the frame.
			u8 Held		: 1;		/// Has this key been pressed for longer than a single frame.
			u8 Mod		: 1;
		};

		struct MouseState
		{
			float X_Position;
			float Y_Position;
			std::array<KeyState, 8> Buttons;
		};

		using KeyMap = std::unordered_map<u32, KeyState>;

		using InputKeyCallbackFunc = std::function<void(int key, int scancode, int action, int mods)>;
		using InputMousePosCallbackFunc = std::function<void(float xpos, float ypos)>;
		using InputMouseButtonCallbackFunc = std::function<void(int button, int action, int mods)>;
		using InputMouseScrollCallbackFunc = std::function<void(float xoffset, float yoffset)>;

		class IS_INPUT InputManager
		{
		public:
			/// [INTERNAL] Call with a valid window to setup all the glfw callbacks.
			static bool InitWithWindow(Graphics::Window* window);
			/// [INTERNAL] Must be called before window update (this resets certain values).
			static void Update();

			static bool IsKeyPressed(u32 key);
			static bool IsKeyReleased(u32 key);
			static bool IsKeyHeld(u32 key);

			static bool IsMouseButtonPressed(u32 key);
			static bool IsMouseButtonReleased(u32 key);
			static bool IsMouseButtonHeld(u32 key);
			static void GetMousePosition(float& x_position, float& y_position);

			static void RegisterKeyCallback(InputKeyCallbackFunc callback)                 { m_key_callback_funcs.push_back(callback); }
			static void RegisterMousePosCallback(InputMousePosCallbackFunc callback)       { m_mouse_pos_callback_funcs.push_back(callback); }
			static void RegisterMouseButtonCallback(InputMouseButtonCallbackFunc callback) { m_mouse_button_callback_funcs.push_back(callback); }
			static void RegisterMouseScrollCallback(InputMouseScrollCallbackFunc callback) { m_mouse_scroll_callback_funcs.push_back(callback); }

		private:
			static bool m_is_initialised;
			static KeyMap m_key_map;
			static MouseState m_mouse_state;

			static std::vector<InputKeyCallbackFunc>			m_key_callback_funcs;
			static std::vector<InputMousePosCallbackFunc>		m_mouse_pos_callback_funcs;
			static std::vector<InputMouseButtonCallbackFunc>	m_mouse_button_callback_funcs;
			static std::vector<InputMouseScrollCallbackFunc>	m_mouse_scroll_callback_funcs;
		};
	}
}