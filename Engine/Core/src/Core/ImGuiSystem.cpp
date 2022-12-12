#include "Core/ImGuiSystem.h"

namespace Insight
{
	namespace Core
	{
		void ImGuiSystem::Initialise()
		{
			ImGui::CreateContext();
			ImGuiIO& io = ImGui::GetIO(); (void)io;
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
			//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
			io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
			//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
			//io.ConfigViewportsNoAutoMerge = true;
			//io.ConfigViewportsNoTaskBarIcon = true;
			// 
			// Setup Dear ImGui style
			ImGui::StyleColorsDark();
			//ImGui::StyleColorsClassic();

			// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
			ImGuiStyle& style = ImGui::GetStyle();
			if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			{
				style.WindowRounding = 0.0f;
				style.Colors[ImGuiCol_WindowBg].w = 1.0f;
			}
			m_state = SystemStates::Initialised;
		}

		void ImGuiSystem::Shutdown()
		{
			ImGui::DestroyContext();
			m_state = SystemStates::Not_Initialised;
		}

		void ImGuiSystem::SetContext(ImGuiContext*& context)
		{
			context = GetCurrentContext();
		}

		ImGuiContext* ImGuiSystem::GetCurrentContext()
		{
			return ImGui::GetCurrentContext();
		}

		void ImGuiSystem::GetAllocatorFunctions(ImGuiMemAllocFunc& allocFunc, ImGuiMemFreeFunc& freeFunc, void*& pUserData)
		{
			return ImGui::GetAllocatorFunctions(&allocFunc, &freeFunc, &pUserData);
		}
	}
}