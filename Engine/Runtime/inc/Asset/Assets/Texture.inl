#pragma once

#include "Texture.h"
#include "Asset/AssetRegistry.h"
#include "Asset/Importers/TextureImporter.h"
#include "FileSystem/FileSystem.h"

#include "Compression/ICompressor.h"

namespace Insight
{
    namespace Serialisation
    {
        template<>
        struct ComplexSerialiser<TextureAssetTextureData, Runtime::TextureAsset::TextureData, Runtime::TextureAsset>
        {
            void operator()(ISerialiser* serialiser, Runtime::TextureAsset::TextureData& textureData, Runtime::TextureAsset* textureAsset) const
            {
                constexpr const char* kTextureData = "TextureData";

                if (serialiser->IsReadMode())
                {
                    serialiser->ReadBinaryBulk(kTextureData, textureData.Bytes);

                    Runtime::TextureImporter textureImporter;

                    Runtime::TextureImportContext context{ std::move(textureData.Bytes) };
                    textureImporter.LoadRaw(context);

                    textureData.Bytes = std::move(context.Data);

                    textureAsset->SetTextureData(textureData.Bytes.data(), textureData.Bytes.size());

                    if (!textureAsset->m_readableWriteable)
                    {
                        textureData.Bytes = {};
                    }
                }
                else
                {
                    if (!textureAsset->m_readableWriteable && textureData.Bytes.empty())
                    {
                        textureData.Bytes = Runtime::AssetRegistry::Instance().LoadAssetData(textureAsset->GetAssetInfo()->GetFullFilePath());
                    }

                    serialiser->WriteBinaryBulk(kTextureData, textureData.Bytes);

                    if (!textureAsset->m_readableWriteable)
                    {
                        textureData.Bytes = {};
                    }
                }
            }
        };

        template<>
        struct ComplexSerialiser<TextureAssetTextureDataQOI, Runtime::TextureAsset::TextureData, Runtime::TextureAsset>
        {
            void operator()(ISerialiser* serialiser, Runtime::TextureAsset::TextureData& textureData, Runtime::TextureAsset* textureAsset) const
            {
                constexpr const char* kTextureData = "TextureData";

                if (serialiser->IsReadMode())
                {
                    Runtime::TextureImporter textureImporter;
                    Runtime::TextureImportContext context
                    {
                        { },
                        textureAsset->m_width,
                        textureAsset->m_height,
                        textureAsset->m_channels
                    };

                    serialiser->ReadBinaryBulk(kTextureData, context.Data);

                    textureImporter.DecompressFromQOI(context);

                    textureAsset->SetTextureData(context.Data.data(), context.Data.size());

                    if (textureAsset->m_readableWriteable)
                    {
                        textureData.Bytes = std::move(context.Data);
                    }
                }
                else
                {
                    Runtime::TextureImporter textureImporter;
                    Runtime::TextureImportContext context
                    {
                        { },
                        textureAsset->m_width,
                        textureAsset->m_height,
                        textureAsset->m_channels
                    };

                    if (!textureAsset->m_readableWriteable && textureData.Bytes.empty())
                    {
                        textureData.Bytes = Runtime::AssetRegistry::Instance().LoadAssetData(textureAsset->GetAssetInfo()->GetFullFilePath());
                        context.Data = std::move(textureData.Bytes);

                        const std::string_view fileExtension = FileSystem::GetExtension(textureAsset->GetAssetInfo()->FileName);
                        textureImporter.LoadRaw(context, fileExtension);
                    }
                    else
                    {
                        context.Data = std::move(textureData.Bytes);
                    }

                    textureImporter.CompressToQOI(context);
                    serialiser->WriteBinaryBulk(kTextureData, context.Data);

                    FileSystem::SaveToFile(context.Data, textureAsset->GetAssetInfo()->FileName);

                    if (textureAsset->m_readableWriteable)
                    {
                        textureImporter.DecompressFromQOI(context);
                        textureData.Bytes = std::move(context.Data);
                    }
                }
            }
        };

        template<>
        struct ComplexSerialiser<TextureAssetTextureDataLZ4, Runtime::TextureAsset::TextureData, Runtime::TextureAsset>
        {
            void operator()(ISerialiser* serialiser, Runtime::TextureAsset::TextureData& textureData, Runtime::TextureAsset* textureAsset) const
            {
                constexpr const char* kTextureData = "TextureData";

                if (serialiser->IsReadMode())
                {
                    Runtime::TextureImporter textureImporter;
                    Runtime::TextureImportContext context
                    {
                        { },
                        textureAsset->m_width,
                        textureAsset->m_height,
                        textureAsset->m_channels
                    };

                    serialiser->ReadBinaryBulk(kTextureData, context.Data);

                    textureImporter.DecompressFromQOI(context);

                    textureAsset->SetTextureData(context.Data.data(), context.Data.size());

                    if (textureAsset->m_readableWriteable)
                    {
                        textureData.Bytes = std::move(context.Data);
                    }
                }
                else
                {
                    Runtime::TextureImporter textureImporter;
                    Runtime::TextureImportContext context
                    {
                        { },
                        textureAsset->m_width,
                        textureAsset->m_height,
                        textureAsset->m_channels
                    };

                    if (!textureAsset->m_readableWriteable && textureData.Bytes.empty())
                    {
                        textureData.Bytes = Runtime::AssetRegistry::Instance().LoadAssetData(textureAsset->GetAssetInfo()->GetFullFilePath());
                        context.Data = std::move(textureData.Bytes);

                        const std::string_view fileExtension = FileSystem::GetExtension(textureAsset->GetAssetInfo()->FileName);
                        textureImporter.LoadRaw(context, fileExtension);
                    }
                    else
                    {
                        context.Data = std::move(textureData.Bytes);
                    }

                    Core::ICompressor* compressor = Core::ICompressor::Create(CompressionTypes::lz4);
                    context.Data = compressor->Compress(context.Data);
                    Delete(compressor);

                    serialiser->WriteBinaryBulk(kTextureData, context.Data);

                    FileSystem::SaveToFile(context.Data, textureAsset->GetAssetInfo()->FileName);

                    if (textureAsset->m_readableWriteable)
                    {
                        textureData.Bytes = std::move(context.Data);
                    }
                }
            }
        };
    }
}