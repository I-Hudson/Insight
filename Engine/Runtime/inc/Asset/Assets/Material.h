#pragma once

#include "Asset/Asset.h"
#include "Asset/Assets/Texture.h"

#include "Core/ReferencePtr.h"
#include "Core/Logger.h"

#include "Generated/Material_reflect_generated.h"

namespace Insight
{
    namespace Runtime
    {
        class ModelAsset;

        enum class MaterialAssetProperty : u8
        {
            Colour_R,
            Colour_G,
            Colour_B,
            Colour_A,

            Opacity,

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

            IS_OBJECT(MaterialAsset);
            IS_SERIALISABLE_H(MaterialAsset);

            void SetTexture(const TextureAssetTypes textureType, Ref<TextureAsset> texture);
            Ref<TextureAsset> GetTexture(const TextureAssetTypes textureType) const;

            void SetProperty(MaterialAssetProperty property, float value);
            float GetProperty(MaterialAssetProperty property) const;
            const std::array<float, static_cast<u32>(MaterialAssetProperty::Count)>& GetProperties() const;

            bool IsTransparent() const;

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

    namespace Serialisation
    {
        struct MaterialTextures {};
        template<>
        struct ComplexSerialiser<MaterialTextures,
            std::array<Ref<Runtime::TextureAsset>, static_cast<u32>(Runtime::TextureAssetTypes::Count)>, Runtime::MaterialAsset>
        {
            void operator()(ISerialiser* serialiser, 
                std::array<Ref<Runtime::TextureAsset>, static_cast<u32>(Runtime::TextureAssetTypes::Count)>& textures, Runtime::MaterialAsset* materialAsset) const
            {
                if (serialiser->IsReadMode())
                {
                    for (u64 i = 0; i < textures.size(); ++i)
                    {
                        std::string textureGuidStr;
                        serialiser->Read(Runtime::TextureAssetTypesToString[i], textureGuidStr);

                        const Core::GUID textureGuid(textureGuidStr);
                        if (textureGuid != Core::GUID::s_InvalidGUID)
                        {
                            Ref<Runtime::Asset> textureAsset = Runtime::AssetRegistry::Instance().LoadAsset(textureGuid);
                            if (textureAsset == nullptr)
                            {
                                IS_LOG_CORE_ERROR("Unable to load texture with guid '{}'.", textureGuidStr.c_str());
                            }
                        }
                    }
                }
                else
                {
                    for (u64 i = 0; i < textures.size(); ++i)
                    {
                        const Ref<Runtime::TextureAsset>& texture = textures[i];
                        Core::GUID textureGuid = Core::GUID::s_InvalidGUID;
                        if (texture)
                        {
                            textureGuid = texture->GetGuid();
                        }
                        serialiser->Write(Runtime::TextureAssetTypesToString[i], textureGuid.ToString());
                    }
                }
            }
        };
    };

    OBJECT_SERIALISER(Runtime::MaterialAsset, 1,
        SERIALISE_BASE(Runtime::Asset, 1, 0)
        SERIALISE_COMPLEX(Serialisation::MaterialTextures, m_textures, 1, 0)
        SERIALISE_ARRAY_PROPERTY(float, m_properties, 1, 0)
    );
}