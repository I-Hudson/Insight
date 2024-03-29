#include "StandaloneModule.h"
#include "Core/ImGuiSystem.h"

#include <imgui.h>

namespace Insight
{
    void StandaloneModule::Initialise(Core::ImGuiSystem* imguiSystem)
    {
		ImGui::SetCurrentContext(imguiSystem->GetCurrentContext());
		ImGuiMemAllocFunc allocFunc;
		ImGuiMemFreeFunc freeFunc;
		void* pUsedData;
		imguiSystem->GetAllocatorFunctions(allocFunc, freeFunc, pUsedData);
		ImGui::SetAllocatorFunctions(allocFunc, freeFunc, pUsedData);
    }
}