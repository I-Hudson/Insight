#pragma once

#include "Asset/AssetInfo.h"
#include "Runtime/Defines.h"
#include "Core/IObject.h"
#include "Core/RefCount.h"

#include <Reflect/Core/Defines.h>

#include "Generated/Asset_reflect_generated.h"

namespace Insight
{
    namespace Runtime
    {
        class AssetRegistry;

        enum class AssetState : u8
        {
            NotLoaded,
            Loading,
            Loaded,
            Unloaded,
            Unloading,
            Invalid
        };

        struct AssetFileHeader
        {
            Core::GUID AssetGuid;
            uint32_t Version = 2;

            uint32_t AssetType;
        };

#define IS_ASSERT(AssetType)\
    static const char* GetStaticAssetFileExtension() { return STRINGIZE_NX(.AssetType); }\
    virtual const char* GetAssetFileExtension() { return STRINGIZE_NX(.AssetType); }

        /// @brief Base class for asset classes. An Asset object is something which can comes from 
        /// a file on disk.
        REFLECT_CLASS();
        class IS_RUNTIME Asset : public Core::RefCount
        {
            REFLECT_GENERATED_BODY()
        public:
            Asset(const AssetInfo* assetInfo);
            virtual ~Asset() override;

            IS_OBJECT(Asset);
            IS_SERIALISABLE_H(Asset);
            IS_ASSERT(Asset);

            const AssetInfo* GetAssetInfo() const;
            AssetState GetAssetState() const;
            bool IsMemoryAsset() const;

            void SetName(const std::string& name);

            std::string GetName() const;
            std::string GetFileName() const;
            std::string GetFilePath() const;
            std::string GetPackageName() const;
            std::string GetPackagePath() const;

        protected:
            virtual void OnUnload() = 0;

            virtual void SerialiseHeader(Serialisation::ISerialiser* serialiser);
            virtual void DeserialiseHeader(Serialisation::ISerialiser* serialiser);

        protected:
            std::string m_name;
            const AssetInfo* m_assetInfo = nullptr;
            std::atomic<AssetState> m_assetState = AssetState::NotLoaded;
            bool m_isMemoryAsset = false;

            // Serialised file header.
            AssetFileHeader m_header;

            friend class AssetRegistry;
        };
    }

    OBJECT_SERIALISER(Runtime::Asset, 1,
        SERIALISE_BASE(IObject, 1, 0)
        SERIALISE_PROPERTY(std::string, m_name, 1, 0)
    );
}