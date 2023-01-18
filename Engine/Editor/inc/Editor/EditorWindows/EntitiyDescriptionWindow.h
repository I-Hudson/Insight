#pragma once

#include "Editor/EditorWindows/IEditorWindow.h"

#include "Core/GUID.h"

namespace Insight
{
    namespace ECS
    {
        class Entity;
        class Component;
    }

    namespace Editor
    {
        class EntitiyDescriptionWindow : public IEditorWindow
        {
        public:
            EntitiyDescriptionWindow();
            EntitiyDescriptionWindow(u32 minWidth, u32 minHeight);
            EntitiyDescriptionWindow(u32 minWidth, u32 minHeight, u32 maxWidth, u32 maxHeight);
            virtual ~EntitiyDescriptionWindow() override;

            EDITOR_WINDOW(EntitiyDescriptionWindow, EditorWindowCategories::Windows);
            virtual void OnDraw() override;

        private:
            ECS::Entity* FindEntity(Core::GUID guid);
            void DrawEntity(ECS::Entity* entity);
            void DrawComponent(ECS::Component* component);
        };
    }
}