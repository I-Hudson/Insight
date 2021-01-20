#include "ispch.h"
#include "SplashScreen.h"

	SplashScreen::SplashScreen(const int& width, const int& height, const std::string& imagePath)
		: m_window(nullptr)
	{

		glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
		glfwWindowHint(GLFW_DECORATED, GL_FALSE);
		m_window = glfwCreateWindow(640,
			480,
			"SplashScreen",
			nullptr, nullptr);


	}

	SplashScreen::~SplashScreen()
	{
	}