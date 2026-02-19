#pragma once

#include "Asset/AssetRegistry.h"

namespace Insight
{
	namespace Editor
	{
		class EditorAssetRegistry : public Runtime::AssetRegistry
		{
		public:
			EditorAssetRegistry();
			virtual ~EditorAssetRegistry() override;


			virtual Ref<Runtime::Asset> LoadAsset(std::string path) override;
		};
	}
}