#include "Editor/Asset/Inspector/TextureAssetInspector.h"

#include "Resource/Loaders/TextureLoader.h"

namespace Insight
{
	namespace Editor
	{
		TextureAssetInspector::TextureAssetInspector()
			: IAssetInspector({ std::begin(Runtime::c_TextureLoaderFileExtensions), std::end(Runtime::c_TextureLoaderFileExtensions) })
		{ }

		TextureAssetInspector::~TextureAssetInspector()
		{ }

		void TextureAssetInspector::Draw(const Runtime::AssetInfo* assetInfo)
		{

		}
	}
}