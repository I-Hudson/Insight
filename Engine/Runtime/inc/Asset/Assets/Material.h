#pragma once

#include "Asset/Asset.h"
#include "Asset/Assets/Texture.h"

#include "Core/ReferencePtr.h"

#include "Generated/Material_reflect_generated.h"

namespace Insight
{
    namespace Runtime
    {
        class ModelAsset;

        enum class MaterialAssetProperty
        {
            Colour_R,
            Colour_G,
            Colour_B,
            Colour_A,

            UV_Tilling_X,
            UV_Tilling_Y,

            Count
        };

        REFLECT_CLASS();
        class IS_RUNTIME MaterialAsset : public Asset
        {
            REFLECT_GENERATED_BODY();
        public:
            MaterialAsset(const AssetInfo* assetInfo);
            virtual ~MaterialAsset() override;

            void SetTexture(const TextureAssetTypes textureType, Ref<TextureAsset> texture);
            Ref<TextureAsset> GetTexture(const TextureAssetTypes textureType) const;

            void SetProperty(MaterialAssetProperty property, float value);
            float GetProperty(MaterialAssetProperty property) const;
            std::array<float, static_cast<u32>(MaterialAssetProperty::Count)> GetProperties() const;

            // Asset - Begin
        protected:
            virtual void OnUnload() override;
            // Asset - End

        private:
            std::array<Ref<TextureAsset>, static_cast<u32>(TextureAssetTypes::Count)> m_textures = { nullptr };
            std::array<float, static_cast<u32>(MaterialAssetProperty::Count)> m_properties = { 0 };

            friend class ModelAsset;
        };
    }
}