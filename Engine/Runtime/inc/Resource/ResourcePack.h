#pragma once

#include "Core/TypeAlias.h"
#include "Core/GUID.h"

#include "Resource/Resource.h"

#include "Serialisation/ISerialisable.h"
#include "Serialisation/Serialiser.h"

#include <string>
#include <fstream>

namespace Insight
{
    namespace Runtime
    {
        class ResourceDatabase;
        class IResourceManager;

        /// @brief A resource pack is an object which is serialised to disk and contains a list of resources from the project.
        class IS_RUNTIME ResourcePack : public Serialisation::ISerialisable
        {
        public:
            struct PackedResource
            {
                IResource* Resource;
                bool IsSerialised = false;
                u64 DataPosition;   // Start position of the resource data on disk.
                u64 DataSize;       // Size of the resource data on disk.
            };
            constexpr static const char* c_Extension = ".isResourcePack";

            ResourcePack(std::string_view path);
            ~ResourcePack();

            void Save();

            std::string_view GetFilePath() const;
            bool IsLoaded() const;
            bool IsUnloaded() const;

            /// @brief Load all resources within this pack.
            /// @return bool
            void LoadAllResources();
            /// @brief Unload all resource within this pack.
            /// @return bool
            void UnloadAllResources();

            std::vector<IResource*> GetAllResources() const;
            /// @brief Return the total amount of resource within the pack.
            /// @return u64
            u64 GetResourceCount() const;
            /// @brief Return the total amount of resources which are loaded.
            /// @return u64
            u64 GetLoadedResourceCount() const;
            /// @brief Return the total amount of resources which are not loaded.
            /// @return u64
            u64 GetUnloadedResourceCount() const;

            void AddResource(IResource* resource);
            void RemoveResource(IResource* resource);
            void RemoveAllResources();

            bool HasResourceId(ResourceId resourceId) const;
            PackedResource GetEntry(ResourceId resourceId) const;


            // -- Begin ISerialisable --
            virtual void Serialise(Serialisation::ISerialiser* serialiser) override;
            virtual void Deserialise(Serialisation::ISerialiser* serialiser) override;
            // -- End ISerialisable --

        private:
            /// @brief Open a file handle to the resource pack file on disk.
            /// @return 
            bool Open();
            void Close();

            bool LoadResource(const PackedResource& packedResource);

        private:
            std::unordered_map<ResourceId, PackedResource> m_resources;
            std::string m_filePath;

            std::fstream m_fileHandle;

            friend class ResourceDatabase;
            friend class IResourceManager;

            IS_SERIALISABLE_FRIEND;
        };
    }

    namespace Serialisation
    {
        struct ResroucePack1 { };
        template<>
        struct ComplexSerialiser<ResroucePack1, void, Runtime::ResourcePack>
        {
            void operator()(ISerialiser* serialiser, Runtime::ResourcePack* resourcePack) const;
        };
    }

    OBJECT_SERIALISER(Runtime::ResourcePack, 1,
        SERIALISE_COMPLEX_THIS(Serialisation::ResroucePack1, 1, 0)
    );
}