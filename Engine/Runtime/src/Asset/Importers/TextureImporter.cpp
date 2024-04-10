#include "Asset/Importers/TextureImporter.h"
#include "Asset/Assets/Texture.h"
#include "Asset/AssetRegistry.h"

#include "FileSystem/FileSystem.h"

#include "Core/Logger.h"
#include "Core/Profiler.h"

#include <stb_image.h>

//#define QOI_IMPLEMENTATION
#include <qoi.h>

namespace Insight
{
    namespace Runtime
    {
        TextureImporter::TextureImporter()
            : IAssetImporter({ ".png", ".jpeg", ".jpg", ".qoi" })
        { }

        Ref<Asset> TextureImporter::Import(const AssetInfo* assetInfo, const std::string_view path) const
        {
            IS_PROFILE_FUNCTION();

            std::vector<Byte> textureData = AssetRegistry::Instance().LoadAssetData(path);
            if (textureData.empty())
            {
                IS_LOG_CORE_ERROR("[TextureImporter::Import] Texture data from path '{}' was empty.", path);
                return Ref<Asset>();
            }

            enum class ImageLoader
            {
                stbi
                , qoi
            };

            ImageLoader imageLoader = ImageLoader::stbi;
            void* textureBuffer = nullptr;
            int width, height, channels;

            {
                IS_PROFILE_SCOPE("stbi_load_from_memory");
                textureBuffer = stbi_load_from_memory(textureData.data(), static_cast<int>(textureData.size()), &width, &height, &channels, STBI_rgb_alpha);
            }

#if 0
            {
                IS_PROFILE_SCOPE("qoi_encode");
                qoi_desc qoiDesc = {};
                qoiDesc.width = width;
                qoiDesc.height = height;
                qoiDesc.channels = 4;
                qoiDesc.colorspace = QOI_SRGB;

                int outLength = -1;
                void* qoiEncode = qoi_encode(textureBuffer, &qoiDesc, &outLength);
                FileSystem::SaveToFile((Byte*)qoiEncode, outLength, std::string(path) + ".qoi", FileType::Binary, true);
            }
#endif

            if (textureBuffer == nullptr)
            {
                imageLoader = ImageLoader::qoi;
                qoi_desc qoiDesc;
                {
                    IS_PROFILE_SCOPE("qoi_decode");
                    textureBuffer = qoi_decode(textureData.data(), static_cast<int>(textureData.size()), &qoiDesc, 4);
                }
                width = qoiDesc.width;
                height = qoiDesc.height;
                channels = static_cast<int>(qoiDesc.channels);
            }

            const u64 textureDataSize = width * height * 4;
            Ref<TextureAsset> texture = Ref<TextureAsset>(New<TextureAsset>(assetInfo));
            texture->m_width = width;
            texture->m_height = height;
            texture->m_depth = 1;
            texture->m_channels = 4;
            texture->m_pixelFormat = PixelFormat::R8G8B8A8_UNorm;
            texture->m_assetState = AssetState::Loaded;
            texture->SetTextureData(textureBuffer, textureDataSize);

            switch (imageLoader)
            {
                case ImageLoader::stbi:
                {
                    stbi_image_free(textureBuffer);
                    break;
                }
                case ImageLoader::qoi:
                {
                    free(textureBuffer);
                    break;
                }
            }

            return texture;
        }
    }
}
