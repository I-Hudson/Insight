#pragma once
#include "Engine/Core/Core.h"
#include "Engine/Core/Log.h"
#include "Engine/Input/Input.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "IconsFontAwesome5.h"

	namespace Module
	{
		class EditorModule;
	}

	namespace Editor
	{
		class EditorWindow
		{
		public:
			EditorWindow() = delete;
			EditorWindow(SharedPtr<Module::EditorModule> editorModule) : m_editorModule(editorModule) {}
			virtual ~EditorWindow() {}

			virtual void Update(const float& deltaTime) = 0;

			const bool IsMouseInBounds()
			{
				float mousePosX, mousePosY;
				Input::GetMousePosition(&mousePosX, &mousePosY);

				auto imguiWindow = ImGui::GetCurrentWindowRead();
				if (imguiWindow)
				{
					auto windowPos = imguiWindow->Pos;
					auto windowSize = imguiWindow->Size;
					return (windowPos.x - (windowSize.x * 0.5f)) <= mousePosX && (windowPos.x + (windowSize.x * 0.5f)) >= mousePosX &&
						(windowPos.y - (windowSize.y * 0.5f)) <= mousePosY && (windowPos.y + (windowSize.y * 0.5f)) >= mousePosY;
				}
				return false;
			}

			const std::string& GetWindowName() { return m_windowName; }

		protected:
			SharedPtr<Module::EditorModule> m_editorModule;
			std::string m_windowName;

#define SET_PANEL_NAME(x) m_windowName = typeid(x).name();
		};
	}
#define ImGui_STORE_TEMP(type, ptr, label, imguiWidget) { type tempV; \
														  tempV = *ptr; \
														  if(imguiWidget(label, &tempV)) \
														  { *ptr = tempV; } }

#define ImGui_STORE_TEMP_NL(type, ptr, imguiWidget) ImGui_STORE_TEMP(type, ptr, "##hidelabel", imguiWidget)

#define ImGui_LABEL_LEFT(label, imguiInput) { ImGui::AlignTextToFramePadding(); ImGui::Text(label); ImGui::SameLine(); imguiInput; }
#define ImGui_LABEL_LEFT_(label) { ImGui::AlignTextToFramePadding(); ImGui::Text(label); ImGui::SameLine(); }