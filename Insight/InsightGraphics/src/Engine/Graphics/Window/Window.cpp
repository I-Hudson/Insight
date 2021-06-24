#include "Engine/Graphics/Window/Window.h"
#include "Engine/Core/Log.h"
#include "Engine/Platform/Platform.h"

namespace Insight::Graphics
{
	GLFWwindow* Window::m_mainWindow;

	void Window::Init(u32 width, u32 height, std::string const& title)
	{
		ASSERT(!glfwInit() && "[Window::Init] GLFW already initialized.");

		glfwSetErrorCallback([](int code, const char* msg)
			{
				 IS_CORE_ERROR("GLFW ERROR: '{0}', '{1}'.", code, msg);
			});

		m_mainWindow = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
		ASSERT(m_mainWindow && "[Window::Init] Window was not created.");

		glfwMakeContextCurrent(m_mainWindow);
	}

	void Window::SetWindowSize(u32 width, u32 height)
	{
		CheckForInit();
	}

	bool Window::ShouldWindowClose()
	{
		return glfwWindowShouldClose(m_mainWindow);
	}

	void Window::CheckForInit()
	{
		ASSERT(glfwInit());
	}

	void Window::Shutdown()
	{
		glfwDestroyWindow(m_mainWindow);
		glfwTerminate();
	}
}