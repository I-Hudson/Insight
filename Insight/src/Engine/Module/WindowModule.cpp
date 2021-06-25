

#include "Engine/Module/WindowModule.h"
#include "Engine/Config/Config.h"
#include "Engine/Event/EventManager.h"
#include "Engine/Event/ApplicationEvent.h"
#include "Engine/Instrumentor/Instrumentor.h"
#include "Engine/Graphics/Graphics.h"


#include "stb_image.h"
#include <glad/glad.h>
#include "GLFW/glfw3.h"


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
	for (auto it = iconPaths.begin(); it != iconPaths.end(); ++it)
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

	if (images.size() > 0)
	{
		glfwSetWindowIcon(m_window, (int)iconPaths.size(), images.data());
	}

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

void Window::HideWindow()
{
	glfwHideWindow(m_window);
}

void Window::ShowWindow()
{
	glfwShowWindow(m_window);
}

void Window::WaitForEvents()
{
	glfwWaitEvents();
}

bool Window::ShouldClose()
{
	return glfwWindowShouldClose(m_window);
}

namespace Insight::Module
{
	Window WindowModule::m_window;

	WindowModule::WindowModule()
	{ }

	WindowModule::~WindowModule()
	{
		glfwDestroyWindow(m_window.m_window);
		glfwTerminate();
	}

	void WindowModule::OnCreate()
	{
		IS_CORE_ASSERT(glfwInit() == 1, "GLFW failed to init!");
		glfwSetErrorCallback([](int errorCode, const char* errorMessage)
		{
			IS_CORE_ERROR("GLFW ERROR CODE: {0}, MESSAGE: {1}", errorCode, errorMessage);
			IS_CORE_ASSERT(false, "GLFW ERROR");
		});

		if (::Graphics::IsVulkan())
		{
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		}
		else if (::Graphics::IsOpenGL())
		{
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		}

		m_window.m_window = glfwCreateWindow(CONFIG_VAL(Config::WindowConfig.WindowWidth),
											 CONFIG_VAL(Config::WindowConfig.WindowHeight),
											 CONFIG_VAL(Config::WindowConfig.WindowTitle).c_str(),
											 nullptr, nullptr);

		m_window.SetIcon({ CONFIG_VAL(Config::WindowConfig.WindowIcon), CONFIG_VAL(Config::WindowConfig.WindowIcon) });
		glfwSetFramebufferSizeCallback(m_window.m_window, [](GLFWwindow* window, int width, int height)
		{
			EventManager::Dispatch(EventType::WindowResize, WindowResizeEvent(width, height));
		});

		if (::Graphics::IsOpenGL())
		{
			glfwMakeContextCurrent(m_window.m_window);

			if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
			{
				IS_CORE_ERROR("Failed to initialize GLAD");

				glViewport(0, 0, CONFIG_VAL(Config::WindowConfig.WindowWidth), CONFIG_VAL(Config::WindowConfig.WindowHeight));
			}
		}

		m_state = ModuleState::Running;
	}

	void WindowModule::Update(const float& deltaTime)
	{
		IS_PROFILE_FUNCTION();

		glfwPollEvents();

		std::stringstream ss;
		ss << "FPS: " << 1.0 / deltaTime;
		m_window.SetTitle(ss.str());
	}
}