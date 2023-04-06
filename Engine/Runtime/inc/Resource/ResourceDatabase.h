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
        class ResourceManager;

        /// @brief Database to store all currently known resources.
        class ResourceDatabase : public Serialisation::ISerialisable
        {
        public:
            using ResourceMap = std::unordered_map<ResourceId, TObjectPtr<IResource>>;
            using ResourceOwningMap = std::unordered_map<ResourceId, TObjectOPtr<IResource>>;

            ResourceDatabase() = default;
            ResourceDatabase(ResourceDatabase const& other);
            ResourceDatabase(ResourceDatabase&& other) = default;
            ~ResourceDatabase() = default;

            IS_SERIALISABLE_H(ResourceDatabase)

            void Initialise();
            void Shutdown();

            TObjectPtr<IResource> AddResource(ResourceId const& resourceId);
            void RemoveResource(TObjectPtr<IResource> resource);
            void RemoveResource(ResourceId const& resourceId);

            TObjectPtr<IResource> GetResource(ResourceId const& resourceId) const;
            TObjectPtr<IResource> GetResourceFromGuid(Core::GUID const& guid) const;
            
            ResourceMap GetResourceMap() const;
            
            bool HasResource(ResourceId const& resourceId) const;
            bool HasResource(TObjectPtr<IResource> resource) const;

            u32 GetResourceCount() const;
            u32 GetLoadedResourceCount() const;
            u32 GetLoadingResourceCount() const;

        private:
            void DeleteResource(TObjectOPtr<IResource>& resource);

            std::vector<ResourceId> GetAllResourceIds() const;
            void OnResourceLoaded(IResource* Resource);
            void OnResourceUnloaded(IResource* Resource);

            TObjectPtr<IResource> CreateDependentResource(ResourceId const& resourceId);
            void RemoveDependentResource(ResourceId const& resourceId);

        private:
            ResourceOwningMap m_resources;
            ResourceOwningMap m_dependentResources;
            u32 m_loadedResourceCount = 0;
            mutable std::mutex m_mutex;

            friend class ResourceManager;
        };
    }

    namespace Serialisation
    {
        struct ResourceDatabase1 {};
        struct ResourceDatabase2 {};

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
    }

    OBJECT_SERIALISER(Runtime::ResourceDatabase, 2,
        SERIALISE_COMPLEX(Serialisation::ResourceDatabase1, m_resources, 1, 2)
        SERIALISE_COMPLEX(Serialisation::ResourceDatabase2, m_resources, 2, 0)
        );
}