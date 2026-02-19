#include "Editor/Asset/EditorAssetRegistry.h"

namespace Insight
{
	namespace Editor
	{
		EditorAssetRegistry::EditorAssetRegistry()
		{ }

		EditorAssetRegistry::~EditorAssetRegistry()
		{ }

		Ref<Runtime::Asset> EditorAssetRegistry::LoadAsset(std::string path)
		{
			return Runtime::AssetRegistry::LoadAsset(path);
		}
	}
}