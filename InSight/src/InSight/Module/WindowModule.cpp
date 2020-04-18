#include "WindowModule.h"

#include "GLFW/glfw3.h"

namespace Insight
{
	GLFWwindow* Window::m_window;

	const int& Window::GetWidth()
	{
		int width;
		glfwGetWindowSize(m_window, &width, nullptr);
		return width;
	}

	const int& Window::GetHeight()
	{
		int height;
		glfwGetWindowSize(m_window, nullptr, &height);
		return height;
	}

	void Window::SetTitle(const std::string_view& title)
	{
		glfwSetWindowTitle(m_window, title.data());
	}

	void Window::SetFullscreen(const bool& fullscreen)
	{
		int xPos, yPos;
		glfwGetWindowPos(m_window, &xPos, &yPos);
		glfwSetWindowMonitor(m_window, glfwGetPrimaryMonitor(), xPos, yPos, GetWidth(), GetHeight(), 60);
	}

	bool Window::IsFullscreen()
	{
		return false;
	}

	bool Window::ShouldClose()
	{
		return glfwWindowShouldClose(m_window);
	}

	namespace Module
	{
		Window* WindowModule::m_window;

		WindowModule::WindowModule()
		{
		}

		WindowModule::~WindowModule()
		{
		}

		void WindowModule::Startup(const ModuleStartupData& startupData)
		{
			m_window = new Window();

			glfwInit();

			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
			glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

			m_window->m_window = glfwCreateWindow(1280, 720, "Test Window", nullptr, nullptr);
		}

		void WindowModule::Shutdown()
		{
			glfwDestroyWindow(m_window->m_window);
			glfwTerminate();

			delete m_window;
		}

		void WindowModule::Update(const float& deltaTime)
		{
			glfwPollEvents();
		}
	}
}
