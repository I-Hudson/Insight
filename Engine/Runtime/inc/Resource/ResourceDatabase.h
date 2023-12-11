#pragma once

#include "Core/Memory.h"

#include "Runtime/Defines.h"
#include "Resource/Resource.h"

#include "Serialisation/Serialiser.h"
#include "Serialisation/ISerialisable.h"

#include <mutex>

namespace Insight
{
    namespace Runtime
    {
        class IResourceManager;
        

        /// @brief Database to store all currently known resources.
        class ResourceDatabase : public Serialisation::ISerialisable
        {
            using MetaFileSerialiser = Serialisation::JsonSerialiser;
        public:
            constexpr static const char* c_MetaFileExtension = ".ismeta";
            using ResourceMap = std::unordered_map<ResourceId, TObjectPtr<IResource>>;
            using ResourceOwningMap = std::unordered_map<ResourceId, TObjectOPtr<IResource>>;

            ResourceDatabase() = default;
            ResourceDatabase(ResourceDatabase const& other);
            ResourceDatabase(ResourceDatabase&& other) = default;
            ~ResourceDatabase() = default;

            IS_SERIALISABLE_H(ResourceDatabase)

            void Initialise();
            void Shutdown();
            void Clear();

            TObjectPtr<IResource> AddResource(ResourceId const& resourceId);
            TObjectPtr<IResource> AddResource(const Core::GUID assetGuid);

            void RemoveResource(TObjectPtr<IResource> resource);
            void RemoveResource(ResourceId const& resourceId);

            void UpdateGuidToResource(TObjectPtr<IResource>& resource);

            TObjectPtr<IResource> GetResource(ResourceId const& resourceId) const;
            TObjectPtr<IResource> GetResourceFromGuid(Core::GUID const& guid) const;
            
            ResourceMap GetResourceMap() const;

            bool HasResource(ResourceId const& resourceId) const;
            bool HasResource(TObjectPtr<IResource> resource) const;
            bool HasResource(const Core::GUID& guid) const;

            u32 GetResourceCount() const;
            u32 GetLoadedResourceCount() const;
            u32 GetLoadingResourceCount() const;

            std::string GetMetaFileForResource(const IResource* resource) const;

        private:
            TObjectPtr<IResource> AddResource(const ResourceId resourceId, bool force);
            TObjectPtr<IResource> AddResource(const Core::GUID assetGuid, ResourceTypeId typeId, bool force);

            void DeleteResource(TObjectOPtr<IResource>& resource);

            std::vector<ResourceId> GetAllResourceIds() const;
            void OnResourceLoaded(IResource* Resource);
            void OnResourceUnloaded(IResource* Resource);

            TObjectPtr<IResource> CreateDependentResource(ResourceId const& resourceId);
            void RemoveDependentResource(ResourceId const& resourceId);

            void LoadMetaFileData(IResource* resource);
            void SaveMetaFileData(IResource* resource, bool overwrite);
            /// @brief Attempt to find any missing resources which couldn't be created
            /// when deserialising the database file. This shouldn't be lent upon 
            /// safety net. Resource/files being moved should be done in editor.
            void FindMissingResources();

        private:
            mutable std::mutex m_resourcesMutex;
            ResourceOwningMap m_resources;

            mutable std::mutex m_dependentResourcesMutex;
            ResourceOwningMap m_dependentResources;

            mutable std::mutex m_guidToResourceMutex;
            std::unordered_map<Core::GUID, TObjectPtr<IResource>> m_guidToResources;

            u32 m_loadedResourceCount = 0;

            /// @brief Use this map in the verify process.
            std::unordered_map<ResourceId, Core::GUID> m_missingResources;

            friend class IResourceManager;
        };
    }

    namespace Serialisation
    {
        struct ResourceDatabase1 {};
        struct ResourceDatabase2 {};
        struct ResourceDatabase3 {};
        struct ResourceDatabase4 {};

        template<>
        struct ComplexSerialiser<ResourceDatabase1, Runtime::ResourceDatabase::ResourceOwningMap, Runtime::ResourceDatabase>
        {
            void operator()(ISerialiser* serialiser, Runtime::ResourceDatabase::ResourceOwningMap& map, Runtime::ResourceDatabase* resourceDatabase) const;
        };
        template<>
        struct ComplexSerialiser<ResourceDatabase2, Runtime::ResourceDatabase::ResourceOwningMap, Runtime::ResourceDatabase>
        {
            void operator()(ISerialiser* serialiser, Runtime::ResourceDatabase::ResourceOwningMap& map, Runtime::ResourceDatabase* resourceDatabase) const;
        };
        template<>
        struct ComplexSerialiser<ResourceDatabase3, Runtime::ResourceDatabase::ResourceOwningMap, Runtime::ResourceDatabase>
        {
            void operator()(ISerialiser* serialiser, Runtime::ResourceDatabase::ResourceOwningMap& map, Runtime::ResourceDatabase* resourceDatabase) const;
        };
        template<>
        struct ComplexSerialiser<ResourceDatabase4, Runtime::ResourceDatabase::ResourceOwningMap, Runtime::ResourceDatabase>
        {
            void operator()(ISerialiser* serialiser, Runtime::ResourceDatabase::ResourceOwningMap& map, Runtime::ResourceDatabase* resourceDatabase) const;
        };
    }

    OBJECT_SERIALISER(Runtime::ResourceDatabase, 5,
        SERIALISE_COMPLEX(Serialisation::ResourceDatabase1, m_resources, 1, 2)
        SERIALISE_COMPLEX(Serialisation::ResourceDatabase2, m_resources, 2, 3)
        SERIALISE_COMPLEX(Serialisation::ResourceDatabase3, m_resources, 3, 4)
        SERIALISE_COMPLEX(Serialisation::ResourceDatabase4, m_resources, 4, 0)
        );
}