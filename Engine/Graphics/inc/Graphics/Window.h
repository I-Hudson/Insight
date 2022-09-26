#pragma once

#include "Defines.h"
#include <string>

#include <glm/ext/vector_int2.hpp>
#include <GLFW/glfw3.h>

namespace Insight
{
	namespace Graphics
	{
		class IS_GRAPHICS Window
		{
		public:

			static Window& Instance()
			{
				static Window instance;
				return instance;
			}

			bool Init(int width = 640, int height = 720, std::string title = "DefaultTitle");
			void Destroy();
			void Update();

			void Rebuild();

			std::string_view GetTitle() const { return m_title; }
			void SetTite(std::string title);

			int GetX() const { return m_position.x; }
			int GetY() const { return m_position.y; }
			glm::ivec2 GetPosition() const { return m_position; }

			void SetX(int x);
			void SetY(int y);
			void SetPosition(glm::ivec2 position);

			int GetWidth() const { return m_size.x; }
			int GetHeight() const { return m_size.y; }
			glm::ivec2 GetSize() const { return m_size; }

			void SetWidth(int width);
			void SetHeight(int height);
			void SetSize(glm::ivec2 size);

			bool ShouldClose() const;

			GLFWwindow* GetRawWindow() const { return m_glfwWindow; }

		private:

			bool m_glfwInit = false;
			std::string m_title;
			glm::ivec2 m_position = {0,0};
			glm::ivec2 m_size = {0,0};
			GLFWwindow* m_glfwWindow = nullptr;
		};
	}
}