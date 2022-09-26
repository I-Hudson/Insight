#pragma once

#include "Core/Defines.h"

struct ImGuiContext;

namespace Insight
{
	namespace Core
	{
		/**
		* Wrapper around ImGui.
		* Used to setup basic imgui, shut it down, and get a common context.
		*/
		class IS_CORE ImGuiSystem
		{
		public:
			/// @brief Initialise ImGui.
			static void Init();
			/// @brief Shutdown ImGui.
			static void Shutdown();
			/// @brief Return the current context.
			static ImGuiContext* GetCurrentContext();
		};
	}
}