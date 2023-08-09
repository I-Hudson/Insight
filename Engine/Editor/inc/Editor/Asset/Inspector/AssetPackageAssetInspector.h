#pragma once

#include "Editor/Asset/IAssetInspector.h"

#include "Generated/AssetPackageAssetInspector_reflect_generated.h"

namespace Insight
{
	namespace Editor
	{
		REFLECT_CLASS()
		class AssetPackageAssetInspector : public IAssetInspector
		{
			REFLECT_GENERATED_BODY();
		public:
			AssetPackageAssetInspector();
			virtual ~AssetPackageAssetInspector() override final;

			virtual void Draw(const Runtime::AssetInfo* assetInfo) override final;
		};
	}
}