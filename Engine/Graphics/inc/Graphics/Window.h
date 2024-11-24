#pragma once

#include "Defines.h"
#include "Core/Singleton.h"

#include "Serialisation/Serialiser.h"

#include "Input/InputSystem.h"

#include "Maths/Vector2.h"

#include <string>
#include <unordered_map>

#include <GLFW/glfw3.h>

namespace Insight
{
	namespace Input
	{
		class InputSystem;
	}

	namespace Graphics
	{
		class Window;

		class IS_GRAPHICS WindowInputs
		{
		public:
			void Clear()
			{
				Inputs.clear();
			}

			GLFWwindow* Window = nullptr;
			std::vector<Input::GenericInput> Inputs;
		};

		class IS_GRAPHICS Window : public Core::Singleton<Window>
		{
		public:

			bool Init(Input::InputSystem* inputSystem, int width = 640, int height = 720, std::string title = "DefaultTitle");
			bool Init(Input::InputSystem* inputSystem, bool startHidden, int width = 640, int height = 720, std::string title = "DefaultTitle");
			void Destroy();
			void Update();

			void Rebuild();

			std::string_view GetTitle() const { return m_title; }
			void SetTite(std::string title);

			void SetIcon(const std::string& file_path);

			float GetAspect() const { return static_cast<float>(GetWidth()) / static_cast<float>(GetHeight()); }

			int GetX() const;
			int GetY() const;
			Maths::IVector2 GetPosition() const;

			void SetX(int x);
			void SetY(int y);
			void SetPosition(Maths::IVector2 position);

			int GetWidth() const;
			int GetHeight() const;
			Maths::IVector2 GetSize() const;

			void SetWidth(int width);
			void SetHeight(int height);
			void SetSize(Maths::IVector2 size);

			void SetFullScreen();
			void SetWindowed();
			bool IsFullScreen() const { return m_isFullScreen; }

			void Show();
			void Hide();

			bool ShouldClose() const;

			GLFWwindow* GetRawWindow() const { return m_glfwWindow; }

		private:
			void SetCallbacks();

		private:
			std::string m_title;
			static std::unordered_map<GLFWwindow*, WindowInputs> m_windowInputs;

			Maths::IVector2 m_fullScreenSize = Maths::IVector2(0, 0);
			Maths::IVector2 m_fullScreenPosition = Maths::IVector2(0, 0);
			Maths::IVector2 m_windowedSize = Maths::IVector2(0, 0);
			Maths::IVector2 m_windowedPosition = Maths::IVector2(0, 0);

			GLFWwindow* m_glfwWindow = nullptr;
			Input::InputSystem* m_inputSystem = nullptr;

			bool m_glfwInit = false;
			bool m_isFullScreen = false;
		};
	}

	//OBJECT_SERIALISER(Graphics::Window, 1,
	//	SERIALISE_PROPERTY(std::string, m_title, 1, 0)
	//	SERIALISE_PROPERTY(bool, m_isFullScreen, 1, 0)
	//	SERIALISE_PROPERTY(glm::ivec2, m_fullScreenSize, 1, 0)
	//	SERIALISE_PROPERTY(glm::ivec2, m_fullScreenPosition, 1, 0)
	//	SERIALISE_PROPERTY(glm::ivec2, m_windowedSize, 1, 0)
	//	SERIALISE_PROPERTY(glm::ivec2, m_windowedPosition, 1, 0)
	//);
}