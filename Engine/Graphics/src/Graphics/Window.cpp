#include "Graphics/Window.h"

#include "Core/Logger.h"
#include "Core/CommandLineArgs.h"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"

#include <stb_image.h>

#include <iostream>

namespace Insight
{
	namespace Graphics
	{
		constexpr char* CMD_START_WINDOW_MINIMISED = "start_window_minimised";

		bool Window::Init(int width, int height, std::string title)
		{
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

			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
			m_glfwWindow = glfwCreateWindow(m_size.x, m_size.y, m_title.c_str(), NULL, NULL);

			int windowPosX, windowPosY;
			glfwGetWindowPos(m_glfwWindow, &windowPosX, & windowPosY);
			m_position = { windowPosX , windowPosY };

			glfwSetWindowPosCallback(m_glfwWindow, [](GLFWwindow* window, int xpos, int ypos)
				{
					Window::Instance().SetPosition({ xpos, ypos });
				});
			glfwSetWindowSizeCallback(m_glfwWindow, [](GLFWwindow* window, int width, int height)
				{
					Window::Instance().SetSize({ width, height });
				});

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

		void Window::Rebuild()
		{
			std::string oldTitle = m_title;
			glm::ivec2 oldPosition = m_position;
			glm::ivec2 oldSize = m_size;

			Destroy();
			Init(oldSize.x, oldSize.y, oldTitle);
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
	}
}