#pragma once
#include "Insight/Core/Core.h"
#include "Insight/Module/Module.h"
#include "Insight/Core/Log.h"
#include "Insight/Memory/MemoryManager.h"
#include <bitset>

#include "Editor/EditorDrawer.h"

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

	namespace Editor
	{
		class EditorWindow;
	}

	namespace Module
	{
		class EditorModule : public Module
			, public TSingleton<EditorModule>
			, public std::enable_shared_from_this<EditorModule>
		{
		public:
			EditorModule();
			~EditorModule() override;

			virtual void OnCreate() override;
			virtual void Update(const float& deltaTime) override;

			template<typename T, typename... Args>
			SharedPtr<T> AddEditorPanel(Args&&... args);

			template<typename T>
			void RemoveEditorPanel();

			template<typename T>
			bool HasEditorPanel();

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

			EditorConfig EditorConfig;

		private:
			Editor::EditorDrawerRegistry m_editorDrawerRegisty;
			std::unordered_map<size_t, SharedPtr<Editor::EditorWindow>> m_editorPanels;
		};

		template<typename T, typename... Args>
		inline SharedPtr<T> EditorModule::AddEditorPanel(Args&&... args)
		{
			IS_CORE_STATIC_ASSERT((std::is_base_of<Editor::EditorWindow, T>::value), "T is not of type EditorWindow");

			if (HasEditorPanel<T>())
			{
				return DynamicPointerCast<T>(m_editorPanels[GetEditorPanelID<T>()]);
			}

			SharedPtr<Editor::EditorWindow> newPanel = CreateSharedPtr<T>(this->shared_from_this(), std::forward<Args>(args)...);
			m_editorPanels[GetEditorPanelID<T>()] = newPanel;

			return DynamicPointerCast<T>(newPanel);
		}

		template<typename T>
		inline void EditorModule::RemoveEditorPanel()
		{
			IS_CORE_STATIC_ASSERT((std::is_base_of<EditorWindow, T>::value), "T is not of type EditorWindow");

			if (Editor::EditorWindow* panel = HasEditorPanel<T>())
			{
				m_editorPanels.erase(GetEditorPanelID<T>());
			}
		}

		template<typename T>
		inline bool EditorModule::HasEditorPanel()
		{
			IS_CORE_STATIC_ASSERT((std::is_base_of<Editor::EditorWindow, T>::value), "T is not of type EditorWindow");
			return m_editorPanels.find(GetEditorPanelID<T>()) != m_editorPanels.end();
		}
	}
#endif