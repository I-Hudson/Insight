#pragma once

#include "Core/Defines.h"
#include "Core/ISysytem.h"

#include <imgui.h>

namespace Insight
{
	namespace Core
	{
		/**
		* Wrapper around ImGui.
		* Used to setup basic imgui, shut it down, and get a common context.
		*/
		class IS_CORE ImGuiSystem : public ISystem
		{
		public:
			IS_SYSTEM(ImGuiSystem);

			virtual void Initialise() override;
			virtual void Shutdown() override;

			void SetContext(ImGuiContext*& context);
			// @brief Return the current context.
			ImGuiContext* GetCurrentContext();
			void GetAllocatorFunctions(ImGuiMemAllocFunc& allocFunc, ImGuiMemFreeFunc& freeFunc, void*& pUserData);
		};
	}
}