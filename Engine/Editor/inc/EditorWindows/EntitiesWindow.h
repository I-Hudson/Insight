#pragma once

#include "EditorWindows/IEditorWindow.h"
#include "Core/GUID.h"

#include <unordered_set>

namespace Insight
{
	namespace ECS
	{
		class Entity;
	}

	namespace Editor
	{
		class EntitiesWindow : public IEditorWindow
		{
		public:
			EntitiesWindow();
			EntitiesWindow(u32 minWidth, u32 minHeight);
			EntitiesWindow(u32 minWidth, u32 minHeight, u32 maxWidth, u32 maxHeight);
			~EntitiesWindow();

			virtual void OnDraw() override;
			EDITOR_WINDOW(EntitiesWindow);

		private:
			void DrawSingleEntity(ECS::Entity* entity);

		private:
			std::unordered_set<Core::GUID> m_selectedEntities;
		};
	}
}