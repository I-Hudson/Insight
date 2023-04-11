#pragma once

#include "Editor/EditorWindows/IEditorWindow.h"
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
		REFLECT_CLASS()
		class EntitiesWindow : public IEditorWindow
		{
		public:
			EntitiesWindow();
			EntitiesWindow(u32 minWidth, u32 minHeight);
			EntitiesWindow(u32 minWidth, u32 minHeight, u32 maxWidth, u32 maxHeight);
			~EntitiesWindow();

			virtual void OnDraw() override;
			EDITOR_WINDOW(EntitiesWindow, EditorWindowCategories::Windows);

			std::unordered_set<Core::GUID> const& GetSelectedEntities() const;

		private:
			void DrawSingleEntity(ECS::Entity* entity);
			void IsEntitySelected(ECS::Entity* entity, bool isSelected);

		private:
			std::unordered_set<Core::GUID> m_selectedEntities;
		};
	}
}