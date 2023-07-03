#pragma once

#include "Core/TypeAlias.h"
#include "Core/GUID.h"

#include "Resource/Resource.h"

#include "Serialisation/ISerialisable.h"
#include "Serialisation/Serialiser.h"

#include <string>

namespace Insight
{
    namespace Runtime
    {
        /// @brief A resource pack is an object which is serialised to disk and contains a list of resources from the project.
        class IS_RUNTIME ResourcePack : public Serialisation::ISerialisable
        {
            struct PackedResource
            {
                IResource* Resource;
                u64 DataPosition;   // Start position of the resource data on disk.
                u64 DataSize;       // Size of the resource data on disk.
            };

        public:
            ResourcePack(std::string_view path);
            ~ResourcePack();

            void Save();

            std::string_view GetFilePath() const;

            /// @brief Load all resources within this pack.
            /// @return bool
            void LoadAllResources();
            /// @brief Unload all resource within this pack.
            /// @return bool
            void UnloadAllResources();

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

            // -- Begin ISerialisable --
            virtual void Serialise(Serialisation::ISerialiser* serialiser) override;
            virtual void Deserialise(Serialisation::ISerialiser* serialiser) override;
            // -- End ISerialisable --

        private:
            std::unordered_map<ResourceId, PackedResource> m_resources;
            std::string m_filePath;

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