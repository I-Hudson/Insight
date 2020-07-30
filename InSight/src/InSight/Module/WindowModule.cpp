#include "ispch.h"

#include "WindowModule.h"
#include "Insight/Config/Config.h"
#include "Insight/Event/EventManager.h"
#include "Insight/Event/ApplicationEvent.h"
#include "Insight/Instrumentor/Instrumentor.h"

#include "stb_image.h"
#ifdef IS_OPENGL
#include <glad/glad.h>
#endif
#include "GLFW/glfw3.h"

namespace Insight
{
	GLFWwindow* Window::m_window;

	const int Window::GetWidth()
	{
		int width;
		glfwGetWindowSize(m_window, &width, nullptr);
		return width;
	}

	const int Window::GetHeight()
	{
		int height;
		glfwGetWindowSize(m_window, nullptr, &height);
		return height;
	}

	void Window::SetTitle(const std::string_view& title)
	{
		glfwSetWindowTitle(m_window, title.data());
	}

	void Window::SetIcon(const std::vector<std::string>& iconPaths)
	{
		std::vector<GLFWimage> images;
		for (auto it = iconPaths.begin(); it != iconPaths.end(); ++it )
		{
			int width, height, channelsInFile;
			unsigned char* data = stbi_load((*it).c_str(), &width, &height, &channelsInFile, 4);
			if (data != nullptr)
			{
				GLFWimage newImage;
				newImage.width = width;
				newImage.height = height;
				newImage.pixels = data;
				images.push_back(newImage);
			}
		}
		glfwSetWindowIcon(m_window, iconPaths.size(), images.data());

		for (auto it = images.begin(); it != images.end(); ++it)
		{
			stbi_image_free((*it).pixels);
		}
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

	void Window::WaitForEvents()
	{
		glfwWaitEvents();
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
			m_window = new Window();

			glfwInit();

#ifdef IS_VULKAN
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
			//glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
#elif defined(IS_OPENGL)
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif // IS_VULKAN


			m_window->m_window = glfwCreateWindow(CONFIG_VAL(Config::WindowConfig.WindowWidth),
				CONFIG_VAL(Config::WindowConfig.WindowHeight),
				CONFIG_VAL(Config::WindowConfig.WindowTitle).c_str(),
				nullptr, nullptr);

			m_window->SetIcon({ CONFIG_VAL(Config::WindowConfig.WindowIcon), CONFIG_VAL(Config::WindowConfig.WindowIcon) });
			glfwSetWindowSizeCallback(m_window->m_window, [](GLFWwindow* window, int width, int height)
				{
					EventManager::Dispatch(EventType::WindowResize, WindowResizeEvent(width, height));
				});

#ifdef IS_OPENGL
			glfwMakeContextCurrent(m_window->m_window);

			if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
			{
				IS_CORE_ERROR("Failed to initialize GLAD");

				glViewport(0, 0, CONFIG_VAL(Config::WindowConfig.WindowWidth), CONFIG_VAL(Config::WindowConfig.WindowHeight));
			}
#endif
		}

		WindowModule::~WindowModule()
		{
			glfwDestroyWindow(m_window->m_window);
			glfwTerminate();

			delete m_window;
		}

		void WindowModule::Update(const float& deltaTime)
		{
			IS_PROFILE_FUNCTION();

			glfwPollEvents();
		}
	}
}
