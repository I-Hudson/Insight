#pragma once

#include "Editor/EditorWindows/IEditorWindow.h"

#include "Editor/Asset/IAssetInspector.h"
#include "Core/GUID.h"

#include "Generated/AssetInspectorWindow_reflect_generated.h"

namespace Insight
{
	namespace Runtime
	{
		class AssetInfo;
	}

	namespace Editor
	{
		REFLECT_CLASS()
		class AssetInspectorWindow : public IEditorWindow
		{
			REFLECT_GENERATED_BODY();
		public:

			AssetInspectorWindow();
			AssetInspectorWindow(u32 minWidth, u32 minHeight);
			AssetInspectorWindow(u32 minWidth, u32 minHeight, u32 maxWidth, u32 maxHeight);
			virtual ~AssetInspectorWindow() override final;

			EDITOR_WINDOW(AssetInspectorWindow, EditorWindowCategories::Windows);

			virtual void Initialise() override final;
			virtual void Shutdown() override final;

			virtual void OnDraw() override final;

			void SetSelectedAssetInfo(const Runtime::AssetInfo* assetInfo);

		private:
			Core::GUID m_selectedAssetInfoGuid;
			AssetInspectorRegistry m_assetInspectorRegisty;
		};
	}
}