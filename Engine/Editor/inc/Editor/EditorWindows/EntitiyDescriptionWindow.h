#pragma once

#include "Editor/EditorWindows/IEditorWindow.h"

#include "Core/GUID.h"

#include "Editor/EditorWindows/Generated/EntitiyDescriptionWindow_reflect_generated.h"

namespace Insight
{
    namespace ECS
    {
        class Entity;
        class Component;
    }

    namespace Editor
    {
        REFLECT_CLASS()
        class EntitiyDescriptionWindow : public IEditorWindow
        {

            REFLECT_GENERATED_BODY()

        public:
            EntitiyDescriptionWindow();
            EntitiyDescriptionWindow(u32 minWidth, u32 minHeight);
            EntitiyDescriptionWindow(u32 minWidth, u32 minHeight, u32 maxWidth, u32 maxHeight);
            virtual ~EntitiyDescriptionWindow() override;

            EDITOR_WINDOW(EntitiyDescriptionWindow, EditorWindowCategories::Windows);
            virtual void OnDraw() override;

        private:
            ECS::Entity* FindEntity(const Core::GUID& guid);
            void DrawEntity(ECS::Entity* entity);
            void DrawComponent(ECS::Component* component);

        private:
            bool m_showAddComponentMenu = false;
            int m_addComponentListBoxIndex = 0;
        };
    }
}