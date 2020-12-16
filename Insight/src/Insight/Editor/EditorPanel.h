#pragma once
#include "Insight/Core.h"
#include "imgui.h"

namespace Insight
{
	namespace Module
	{
		class EditorModule;
	}

	namespace Editor
	{
		class EditorPanel
		{
		public:
			EditorPanel(SharedPtr<Module::EditorModule> editorModule) : m_editorModule(editorModule) {}
			virtual ~EditorPanel() {}

			virtual void Update(const float& deltaTime) = 0;

			const std::string& GetPanelName() { return m_panelName; }

		protected:
			SharedPtr<Module::EditorModule> m_editorModule;
			std::string m_panelName;

#define SET_PANEL_NAME(x) m_panelName = typeid(x).name();
		};
	}
}
#define ImGui_STORE_TEMP(type, ptr, label, imguiWidget) { type tempV; \
														  tempV = *ptr; \
														  if(imguiWidget(label, &tempV)) \
														  { *ptr = tempV; } }

#define ImGui_STORE_TEMP_NL(type, ptr, imguiWidget) ImGui_STORE_TEMP(type, ptr, "##hidelabel", imguiWidget)

#define ImGui_LABEL_LEFT(label, imguiInput) { ImGui::AlignTextToFramePadding(); ImGui::Text(label); ImGui::SameLine(); imguiInput; }
#define ImGui_LABEL_LEFT_(label) { ImGui::AlignTextToFramePadding(); ImGui::Text(label); ImGui::SameLine(); }