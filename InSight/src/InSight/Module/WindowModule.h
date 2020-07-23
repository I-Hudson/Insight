#pragma once

#include "Insight/Core.h"
#include "Insight/Module/Module.h"
#include "Insight/Templates/TSingleton.h"
#include "Insight/Config/CVar.h"

struct GLFWwindow;

namespace Insight
{
	class IS_API Window
	{
	public:
		static const int GetWidth();
		static const int GetHeight();

		static void SetTitle(const std::string_view& title);
		static void SetIcon(const std::vector<std::string>& iconPaths);
		static void SetFullscreen(const bool& fullscreen = true);
		static bool IsFullscreen();

		static void WaitForEvents();
		static bool ShouldClose();

		static GLFWwindow* m_window;
	};

	namespace Module
	{
		class IS_API WindowModule : public Module
		{
		public:
			WindowModule(ModuleStartupData& startupData = ModuleStartupData());
			virtual ~WindowModule() override;

			virtual void Update(const float& deltaTime) override;

			static const Window* GetWindow() { return m_window; }

			struct WindowConfig
			{
				CVar<int> WindowWidth{ "window_width", 1024 };
				CVar<int> WindowHeight{ "window_height", 720 };
				CVarString WindowIcon{ "window_icon", "testIcon.png" };
				CVarString WindowTitle{ "window_title", "Test Window" };
			};

		private:
			static Window* m_window;
		};
	}
}

