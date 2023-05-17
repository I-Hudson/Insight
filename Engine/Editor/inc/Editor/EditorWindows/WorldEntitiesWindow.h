#pragma once

#include "Editor/EditorWindows/IEditorWindow.h"
#include "Core/GUID.h"

#include "Editor/EditorWindows/Generated/WorldEntitiesWindow_reflect_generated.h"

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
		class WorldEntitiesWindow : public IEditorWindow
		{
			REFLECT_GENERATED_BODY()

		public:
			WorldEntitiesWindow();
			WorldEntitiesWindow(u32 minWidth, u32 minHeight);
			WorldEntitiesWindow(u32 minWidth, u32 minHeight, u32 maxWidth, u32 maxHeight);
			virtual ~WorldEntitiesWindow() override;

			virtual void Initialise() override;
			virtual void Shutdown() override;

			virtual void OnDraw() override;
			EDITOR_WINDOW(WorldEntitiesWindow, EditorWindowCategories::Windows);

			std::unordered_set<Core::GUID> const& GetSelectedEntities() const;

		private:
			void DrawSingleEntity(ECS::Entity* entity);
			void IsEntitySelected(ECS::Entity* entity, bool isSelected);

		private:
			std::unordered_set<Core::GUID> m_selectedEntities;
		};
	}
}