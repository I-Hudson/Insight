#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Module/Module.h"
#include "Engine/Templates/TSingleton.h"
#include "Engine/Config/CVar.h"

struct GLFWwindow;

class Window
{
public:
	static const int GetWidth();
	static const int GetHeight();

	static void SetTitle(const std::string_view& title);
	static void SetIcon(const std::vector<std::string>& iconPaths);
	static void SetFullscreen(const bool& fullscreen = true);
	static bool IsFullscreen();

	static void HideWindow();
	static void ShowWindow();

	static void WaitForEvents();
	static bool ShouldClose();

	static GLFWwindow* m_window;
};


namespace Insight::Module
{
	class IS_API WindowModule : public Module
	{
	public:
		WindowModule();
		virtual ~WindowModule() override;

		virtual void OnCreate() override;
		virtual void Update(const float& deltaTime) override;

		static const Window* GetWindow() { return &m_window; }

		struct WindowConfig
		{
			CVar<int> WindowWidth{ "window_width", 1024 };
			CVar<int> WindowHeight{ "window_height", 720 };
			CVarString WindowIcon{ "window_icon", "testIcon.png" };
			CVarString WindowTitle{ "window_title", "Test Window" };
		};

	private:
		static Window m_window;
	};
}