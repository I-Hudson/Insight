#pragma once

#include "Engine/Core/Common.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Insight::GraphicsAPI::Vulkan
{
	class GPUSwapchainVulkan;
}

namespace Insight::Graphics
{
	class Window
	{
	public:

		static void Init(u32 width, u32 height, std::string const& title);
		static GLFWwindow* GetMainWindow() { CheckForInit(); return m_mainWindow; }
		static void SetWindowSize(u32 width, u32 height);
		static bool ShouldWindowClose();

	private:
		static void CheckForInit();
		static void Shutdown();

	private:
		static GLFWwindow* m_mainWindow;
		friend GraphicsAPI::Vulkan::GPUSwapchainVulkan;
	};
}