#pragma once

#include "Insight/Core.h"
#include "Insight/Module/Module.h"

#include <string_view>

struct GLFWwindow;

namespace Insight
{
	class Window
	{
	public:
		static const int& GetWidth();
		static const int& GetHeight();

		static void SetTitle(const std::string_view& title);
		static void SetFullscreen(const bool& fullscreen = true);
		static bool IsFullscreen();

		static bool ShouldClose();

		static GLFWwindow* m_window;
	};

	namespace Module
	{
		class IS_API WindowModule :public Module
		{
		public:
			WindowModule();
			virtual ~WindowModule() override;

			virtual void Startup(const ModuleStartupData& startupData = ModuleStartupData()) override;
			virtual void Shutdown() override;
			virtual void Update(const float& deltaTime) override;

			static const Window* GetWindow() { return m_window; }

		private:
			static Window* m_window;
		};
	}
}

