#pragma once
#include "Insight/Core.h"
#include "Module.h"
#include "Insight/Log.h"
#include "Insight/Memory/MemoryManager.h"
#include <bitset>

#include "Insight/Editor/EditorDrawer.h"

#include <glm/glm.hpp>

#if defined(IS_EDITOR)

inline size_t GetEditorPanelID()
{
	static size_t lastID = 0;
	return lastID++;
}

template<typename T>
inline size_t GetEditorPanelID() noexcept
{
	static size_t typeID = GetEditorPanelID();
	return typeID;
}

namespace Insight
{
	namespace Editor
	{
		class EditorPanel;
	}

	namespace Module
	{
		class EditorModule : public Module, public TSingleton<EditorModule>
		{
		public:
			EditorModule();
			~EditorModule() override;

			virtual void Update(const float& deltaTime) override;

			template<typename T, typename... Args>
			T* AddEditorPanel(Args&&... args);

			template<typename T>
			void RemoveEditorPanel();

			template<typename T>
			T* HasEditorPanel();

			struct EditorConfig
			{
				CVar<glm::vec4> WindowBG{ "window_bg", glm::vec4() };

				CVar<glm::vec4> Header{ "header", glm::vec4() };
				CVar<glm::vec4> HeaderHovered{ "header_hovered", glm::vec4() };
				CVar<glm::vec4> HeaderActive{ "header_active", glm::vec4() };


				CVar<glm::vec4> Button{ "button", glm::vec4() };
				CVar<glm::vec4> ButtonHovered{ "button_hovered", glm::vec4() };
				CVar<glm::vec4> ButtonActive{ "button_active", glm::vec4() };


				CVar<glm::vec4> FrameBg{ "frame_bg", glm::vec4() };
				CVar<glm::vec4> FrameBgHovered{ "frame_bg_hovered", glm::vec4() };
				CVar<glm::vec4> FrameBgActive{ "frame_bg_active", glm::vec4() };


				CVar<glm::vec4> Tab{ "tab", glm::vec4() };
				CVar<glm::vec4> TabHovered{ "tab_hovered", glm::vec4() };
				CVar<glm::vec4> TabActive{ "tab_active", glm::vec4() };
				CVar<glm::vec4> TabUnfocused{ "tab_unfocused", glm::vec4() };
				CVar<glm::vec4> TabUnfocusedActive{ "tab_unfocused_active", glm::vec4() };


				CVar<glm::vec4> TitleBg{ "title_bg", glm::vec4() };
				CVar<glm::vec4> TitleBgActive{ "title_bg_active", glm::vec4() };
				CVar<glm::vec4> TitleBgCollapsed{ "title_bg_collapsed", glm::vec4() };

			};

		private:
			Editor::EditorDrawerRegistry m_editorDrawerRegisty;
			std::unordered_map<size_t, Editor::EditorPanel*> m_editorPanels;
		};

		template<typename T, typename... Args>
		inline T* EditorModule::AddEditorPanel(Args&&... args)
		{
			IS_CORE_STATIC_ASSERT((std::is_base_of<Editor::EditorPanel, T>::value), "T is not of type EditorPanel");

			if (T* panel = HasEditorPanel<T>())
			{
				return panel;
			}

			Editor::EditorPanel* newPanel = NEW_ON_HEAP(T, this, std::forward<Args>(args)...);
			m_editorPanels[GetEditorPanelID<T>()] = newPanel;

			return static_cast<T*>(newPanel);
		}

		template<typename T>
		inline void EditorModule::RemoveEditorPanel()
		{
			IS_CORE_STATIC_ASSERT((std::is_base_of<EditorPanel, T>::value), "T is not of type EditorPanel");

			if (Editor::EditorPanel* panel = HasEditorPanel<T>())
			{
				m_editorPanels.erase(GetEditorPanelID<T>());
				DELETE_ON_HEAP(panel);
			}
		}

		template<typename T>
		inline T* EditorModule::HasEditorPanel()
		{
			IS_CORE_STATIC_ASSERT((std::is_base_of<Editor::EditorPanel, T>::value), "T is not of type EditorPanel");
			return static_cast<T*>(m_editorPanels[GetEditorPanelID<T>()]);
		}
	}
}
#endif