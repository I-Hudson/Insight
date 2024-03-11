#include "Resource/Texture.h"
#include "Resource/Texture.inl"

#include "Graphics/RHI/RHI_Texture.h"

#include "Serialisation/Archive.h"
#include "FileSystem/FileSystem.h"

#ifndef IS_MONOLITH
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#endif
#include <stb_image.h>
#include <stb_image_write.h>

//#define QOI_IMPLEMENTATION
#include <qoi.h>

namespace Insight
{
    namespace Runtime
    {
        IS_SERIALISABLE_CPP(TextureMetaData);
        IS_SERIALISABLE_CPP(TextureSourceData);
        IS_SERIALISABLE_CPP(Texture);

        Texture::Texture(std::string_view filePath)
            : IResource(filePath)
        {
            m_metaData.DiskFormat = IsEngineFormat() ? TextureDiskFormat::QOI : TextureDiskFormat::Other;
        }

        u32 Texture::GetWidth() const
        {
            return m_width;
        }

        u32 Texture::GetHeight() const
        {
            return m_height;
        }

        u32 Texture::GetDepth() const
        {
            return m_depth;
        }

        PixelFormat Texture::GetFormat() const
        {
            return m_metaData.PixelFormat;
        }

        void Texture::SetAlpha(bool value)
        {
            m_rhi_texture->m_hasAlpha = value;
        }

        Graphics::RHI_Texture* Texture::GetRHITexture() const
        {
            return m_rhi_texture;
        }

        ResourceId Texture::ConvertToEngineFormat()
        {
            ASSERT(!IsEngineFormat() && m_metaData.DiskFormat == TextureDiskFormat::Other);
            std::lock_guard lock(m_mutex);

            std::string sourceFilePath = m_file_path;
            std::string engineFormatFilePath = FileSystem::ReplaceExtension(m_file_path, GetResourceFileExtension());

            Byte* sourceData = m_rawDataPtr;
            u64 sourceDataSize = m_dataSize;
            TextureDiskFormat sourceDiskFormat = m_metaData.DiskFormat;

            // Image data as raw bytes ready to use.
            void* rawImageData = m_rawDataPtr;

            int imageWidth, imageHeight, imageChannels;
            if (m_diskPackedType == TextureDiskPackedType::Packed)
            {
                // If our data is packed then unpack it from its format to raw bytes.
                rawImageData = nullptr;
                rawImageData = stbi_load_from_memory(sourceData, static_cast<int>(sourceDataSize), &imageWidth, &imageHeight, &imageChannels, STBI_rgb_alpha);
                imageChannels = STBI_rgb_alpha;
            }

            qoi_desc qoiDecs;
            qoiDecs.width = imageWidth;
            qoiDecs.height = imageHeight;
            qoiDecs.channels = imageChannels;
            qoiDecs.colorspace = QOI_SRGB;

            int qoiLength = 0;
            // QOI image data formatted.
            void* qoiImageData = qoi_encode(rawImageData, &qoiDecs, &qoiLength);

            if (m_diskPackedType == TextureDiskPackedType::Packed)
            {
                stbi_image_free(rawImageData);
                rawImageData = nullptr;
            }

            // This is very much a hack. Maybe there should be some form of converter function
            // which takes a source asset and converts it? Maybe the resource system should 
            // help with this and needs a rethink.
            m_file_path = engineFormatFilePath;
            m_rawDataPtr = static_cast<Byte*>(qoiImageData);
            m_dataSize = static_cast<u64>(qoiLength);
            m_metaData.DiskFormat = TextureDiskFormat::QOI;

            IResource::ResourceSerialiserType serialiser;
            Serialise(&serialiser);

            std::free(qoiImageData);
            m_rawDataPtr = sourceData;
            m_dataSize = sourceDataSize;
            m_metaData.DiskFormat = sourceDiskFormat;

            Archive archive(m_file_path, ArchiveModes::Write);
            archive.Write(serialiser.GetSerialisedData());
            archive.Close();

            return ResourceId(engineFormatFilePath, GetResourceTypeId());
        }
    }
}