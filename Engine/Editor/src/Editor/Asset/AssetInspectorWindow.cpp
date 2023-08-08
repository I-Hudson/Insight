#include "Editor/Asset/AssetInspectorWindow.h"
#include "Editor/Asset/IAssetInspector.h"

#include "Asset/AssetRegistry.h"
#include "Asset/AssetInfo.h"

#include "FileSystem/FileSystem.h"

namespace Insight
{
	namespace Editor
	{
		AssetInspectorWindow::AssetInspectorWindow()
			: IEditorWindow()
		{ }
		
		AssetInspectorWindow::AssetInspectorWindow(u32 minWidth, u32 minHeight)
			: IEditorWindow(minWidth, minHeight)
		{ }
		
		AssetInspectorWindow::AssetInspectorWindow(u32 minWidth, u32 minHeight, u32 maxWidth, u32 maxHeight)
			: IEditorWindow(minWidth, minHeight, maxWidth, maxHeight)
		{ }

		AssetInspectorWindow::~AssetInspectorWindow()
		{ }

		void AssetInspectorWindow::Initialise()
		{
			m_assetInspectorRegisty.Initialise();
		}

		void AssetInspectorWindow::Shutdown()
		{
			m_assetInspectorRegisty.Shutdown();
		}

		void AssetInspectorWindow::OnDraw()
		{
			const Runtime::AssetInfo* assetInfo = Runtime::AssetRegistry::Instance().GetAsset(m_selectedAssetInfoGuid);
			if (!assetInfo)
			{
				return;
			}

			IAssetInspector* inspector = AssetInspectorRegistry::Instance().GetInspectorFromExtension(FileSystem::GetExtension(assetInfo->FileName));
			if (inspector)
			{
				inspector->Draw(assetInfo);
			}
		}

		void AssetInspectorWindow::SetSelectedAssetInfo(const Runtime::AssetInfo* assetInfo)
		{
			m_selectedAssetInfoGuid = assetInfo->Guid;
		}
	}
}