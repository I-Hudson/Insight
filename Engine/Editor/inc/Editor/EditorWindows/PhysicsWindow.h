#pragma once

#include "Editor/EditorWindows/IEditorWindow.h"

#include "Generated/PhysicsWindow_reflect_generated.h"

namespace Insight
{
    namespace Runtime
    {
        class IAssetPackage;
    }

    namespace Editor
    {
        REFLECT_CLASS()
        class PhysicsWindow : public IEditorWindow
        {
            REFLECT_GENERATED_BODY()
        public:
            PhysicsWindow();
            PhysicsWindow(u32 minWidth, u32 minHeight);
            PhysicsWindow(u32 minWidth, u32 minHeight, u32 maxWidth, u32 maxHeight);
            virtual ~PhysicsWindow() override final;

            EDITOR_WINDOW(PhysicsWindow, EditorWindowCategories::Windows);
            virtual void OnDraw() override final;
        };
    }
}