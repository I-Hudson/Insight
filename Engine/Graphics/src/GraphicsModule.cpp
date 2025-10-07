#include "GraphicsModule.h"
#include "Core/ImGuiSystem.h"
#include "Core/Logger.h"
#include "Core/Asserts.h"

namespace Insight
{
	void GraphicsModule::Initialise(Core::ImGuiSystem* imguiSystem)
	{
#ifndef IS_MONOLITH
		SET_IMGUI_CURRENT_CONTEXT();
		SET_SPDLOG_LOGGERS();
#endif
	}

	void GraphicsModule::Shutdown(Core::ImGuiSystem* imguiSystem)
	{
		ASSERT(ImGui::GetCurrentContext() == imguiSystem->GetCurrentContext());
	}
}