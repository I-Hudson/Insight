#pragma once
#include "Insight/Core.h"

namespace Insight
{
	namespace Module
	{
		class EditorModule;
	}

	class EditorPanel
	{
	public:
		EditorPanel(const Module::EditorModule* editorModule) : m_editorModule(editorModule) { }
		virtual ~EditorPanel() { }

		virtual void Update(const float& deltaTime) = 0;

		const std::string& GetPanelName() { return m_panelName; }

	protected:
		const Module::EditorModule* m_editorModule;
		std::string m_panelName;

#define SET_PANEL_NAME(x) m_panelName = typeid(x).name();
	};
}