#pragma once

#include "Insight/Core/Core.h"
#include "GLFW/glfw3.h"

/*
	Due to this being GLFW this must be created on the same thread as glfw.
*/

namespace Insight
{
	class SplashScreen
	{
		SplashScreen() = delete;
		SplashScreen(const int& width, const int& height, const std::string& imagePath);
		~SplashScreen();

	private:
		GLFWwindow* m_window;
	};
}