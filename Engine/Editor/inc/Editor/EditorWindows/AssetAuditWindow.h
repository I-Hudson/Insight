#pragma once

#include "Editor/EditorWindows/IEditorWindow.h"

#include "Generated/AssetAuditWindow_reflect_generated.h"

namespace Insight
{
    namespace Runtime
    {
        class IAssetPackage;
    }

    namespace Editor
    {
        REFLECT_CLASS()
        class AssetAuditWindow : public IEditorWindow
        {
            REFLECT_GENERATED_BODY()
        public:
            AssetAuditWindow();
            AssetAuditWindow(u32 minWidth, u32 minHeight);
            AssetAuditWindow(u32 minWidth, u32 minHeight, u32 maxWidth, u32 maxHeight);
            virtual ~AssetAuditWindow() override final;

            EDITOR_WINDOW(AssetAuditWindow, EditorWindowCategories::Windows);
            virtual void OnDraw() override final;

        private:
            Runtime::IAssetPackage* m_selectedAssetPackage = nullptr;
        };
    }
}