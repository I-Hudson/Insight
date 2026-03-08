#pragma once

#include "Asset/AssetRegistry.h"

#include <unordered_map>
#include <Graphics/PixelFormat.h>

namespace Insight
{
	namespace Editor
	{
		class TextureImportSettings : public Runtime::IAssetSubMetaData
		{
		public:
			virtual ~TextureImportSettings() override { }

			IS_SERIALISABLE_H(TextureImportSettings);
			IS_ASSET_SUBMETA_DATA_TYPE(TextureImportSettings);

			PixelFormat Format = PixelFormat::Unknown;
			bool IsReadable = false;;
		};

		class EditorAssetRegistry : public Runtime::AssetRegistry
		{
		public:
			EditorAssetRegistry();
			virtual ~EditorAssetRegistry() override;

			// Begin - ISystem -
			virtual void Initialise() override;
			virtual void Shutdown() override;
			// End - ISystem -

			void InitialiseCache();

			virtual Ref<Runtime::Asset> LoadAsset(std::string path) override;

		private:
			std::string_view GetCachedRootFolder() const { return m_cacheRootFolder; }
			Ref<Runtime::Asset> LoadCachedAsset(const Runtime::AssetInfo* assetInfo);

		private:
			std::string m_cacheRootFolder;
		};
	}

	OBJECT_SERIALISER(Editor::TextureImportSettings, 1,
		SERIALISE_PROPERTY(PixelFormat, Format, 1, 0)
		SERIALISE_PROPERTY(bool, IsReadable, 1, 0)
	);
}