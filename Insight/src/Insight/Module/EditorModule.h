#pragma once
#include "Insight/Core.h"
#include "Module.h"
#include "Insight/Log.h"
#include "Insight/Memory/MemoryManager.h"
#include <bitset>

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
	class EditorPanel;

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

		private:
			std::unordered_map<size_t, EditorPanel*> m_editorPanels;
		};

		template<typename T, typename... Args>
		inline T* EditorModule::AddEditorPanel(Args&&... args)
		{
			IS_CORE_STATIC_ASSERT(EditorPanel, T, "T is not of type EditorPanel");

			if (T* panel = HasEditorPanel<T>())
			{
				return panel;
			}
			
			EditorPanel* newPanel = NEW_ON_HEAP(T, this, std::forward<Args>(args)...);
			m_editorPanels[GetEditorPanelID<T>()] = newPanel;
			
			return static_cast<T*>(newPanel);
		}

		template<typename T>
		inline void EditorModule::RemoveEditorPanel()
		{
			IS_CORE_STATIC_ASSERT(EditorPanel, T, "T is not of type EditorPanel");

			if (EditorPanel* panel = HasEditorPanel<T>())
			{
				m_editorPanels.erase(GetEditorPanelID<T>());
				DELETE_ON_HEAP(panel);
			}
		}

		template<typename T>
		inline T* EditorModule::HasEditorPanel()
		{
			IS_CORE_STATIC_ASSERT(EditorPanel, T, "T is not of type EditorPanel");
			return static_cast<T*>(m_editorPanels[GetEditorPanelID<T>()]);
		}
	}
}
