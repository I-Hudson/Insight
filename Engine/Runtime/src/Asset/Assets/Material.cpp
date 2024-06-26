#include "Asset/Assets/Material.h"

namespace Insight
{
    namespace Runtime
    {
        MaterialAsset::MaterialAsset(const AssetInfo* assetInfo)
            : Asset(assetInfo)
        { }

        MaterialAsset::~MaterialAsset()
        { }

        void MaterialAsset::SetTexture(const TextureAssetTypes textureType, Ref<TextureAsset> texture)
        {
            m_textures[static_cast<u32>(textureType)] = texture;
        }

        Ref<TextureAsset> MaterialAsset::GetTexture(const TextureAssetTypes textureType) const
        {
            return m_textures[static_cast<u32>(textureType)];
        }

        void MaterialAsset::SetProperty(MaterialAssetProperty property, float value)
        {
            m_properties[static_cast<u32>(property)] = value;
        }

        float MaterialAsset::GetProperty(MaterialAssetProperty property) const
        {
            return m_properties[static_cast<u32>(property)];
        }

        const std::array<float, static_cast<u32>(MaterialAssetProperty::Count)>& MaterialAsset::GetProperties() const
        {
            return m_properties;
        }

        bool MaterialAsset::IsTransparent() const
        {
            return m_properties[static_cast<u32>(MaterialAssetProperty::Opacity)] < 1.0f;
        }

        void MaterialAsset::OnUnload()
        {
            for (size_t i = 0; i < m_textures.size(); ++i)
            {
                m_textures[i] = nullptr;
            }
            for (size_t i = 0; i < m_properties.size(); ++i)
            {
                m_properties[i] = 0.0f;
            }
        }
    }
}