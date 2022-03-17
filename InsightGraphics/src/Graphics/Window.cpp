#include "Graphics/Window.h"
#include <iostream>

namespace Insight
{
	namespace Graphics
	{
		bool Window::Init(int width, int height, std::string title)
		{
			if (m_glfwInit)
			{
				std::cout << "[Window::Init] Init already called." << '\n';
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
					std::cout << "ErrorCode: " << error_code << ", Description: " << description << '\n';
				});

			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
			m_glfwWindow = glfwCreateWindow(m_size.x, m_size.y, m_title.c_str(), NULL, NULL);

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
				glfwTerminate();
				m_glfwInit = false;
			}
		}

		void Window::Update()
		{
			glfwPollEvents();
		}

		void Window::SetTite(std::string title)
		{
			m_title = std::move(title);
			glfwSetWindowTitle(m_glfwWindow, m_title.c_str());
		}

		void Window::SetX(int x)
		{

		}

		void Window::SetY(int y)
		{
		}

		void Window::SetPosition(glm::ivec2 position)
		{
		}

		void Window::SetWidth(int width)
		{
		}

		void Window::SetHeight(int height)
		{
		}

		void Window::SetSize(glm::ivec2 size)
		{
		}

		bool Window::ShouldClose() const
		{
			if (!m_glfwInit || m_glfwWindow == nullptr)
			{
				return true;
			}
			return glfwWindowShouldClose(m_glfwWindow);
		}
	}
}