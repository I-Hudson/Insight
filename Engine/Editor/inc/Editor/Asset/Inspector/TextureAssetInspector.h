#pragma once

#include "Editor/Asset/IAssetInspector.h"

#include "Generated/TextureAssetInspector_reflect_generated.h"

namespace Insight
{
	namespace Editor
	{
		REFLECT_CLASS()
		class TextureAssetInspector : public IAssetInspector
		{
			REFLECT_GENERATED_BODY();
		public:
			TextureAssetInspector();
			virtual ~TextureAssetInspector() override final;

			virtual void Draw(const Runtime::AssetInfo* assetInfo) override final;
		};
	}
}