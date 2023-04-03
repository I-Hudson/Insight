#pragma once

#include "Core/Defines.h"
#include "Core/ISysytem.h"

#include <imgui.h>
#include <implot.h>

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
			ImGuiContext* GetCurrentContext() const;
			void GetAllocatorFunctions(ImGuiMemAllocFunc& allocFunc, ImGuiMemFreeFunc& freeFunc, void*& pUserData);

			void SetImPlotContext(ImPlotContext*& context);
			ImPlotContext* GetImPlotContext() const;
		};
	}

#define SET_IMGUI_CURRENT_CONTEXT() ImGui::SetCurrentContext(imguiSystem->GetCurrentContext()); \
	ImGuiMemAllocFunc allocFunc;																\
	ImGuiMemFreeFunc freeFunc;																	\
	void* pUsedData;																			\
	imguiSystem->GetAllocatorFunctions(allocFunc, freeFunc, pUsedData);							\
	ImGui::SetAllocatorFunctions(allocFunc, freeFunc, pUsedData);								\
	ImPlot::SetCurrentContext(imguiSystem->GetImPlotContext())
//#define SET_IMPLOT_CURRENT_CONTEXT() ImPlot::
}