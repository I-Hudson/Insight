#pragma once

#include "Core/TypeAlias.h"
#include "Algorithm/Hash.h"

#include "Resource/ResourceTypeId.h"

#include <string>

namespace Insight
{
    namespace Runtime
    {
        class IS_RUNTIME ResourceId : public Serialisation::ISerialisable
        {
        public:
            ResourceId();
            ResourceId(std::string_view path, ResourceTypeId typeId);
            ResourceId(const ResourceId& other);
            ResourceId(ResourceId&& other);
            ~ResourceId();

            IS_SERIALISABLE_H(ResourceId)

            const std::string& GetPath() const;
            const ResourceTypeId& GetTypeId() const;
            //u64 GetId() const;

            operator bool() const;
            bool operator==(ResourceId const& other) const;
            bool operator!=(ResourceId const& other) const;

            ResourceId& operator=(ResourceId const& other);
            ResourceId& operator=(ResourceId&& other);

        private:
            std::string m_path;
            ResourceTypeId m_typeId;
            u64 m_id = 0;

            template<typename>
            friend struct ::Insight::Serialisation::SerialiserObject;
        };
    }

    OBJECT_SERIALISER(Runtime::ResourceId, 2,
        SERIALISE_PROPERTY(std::string, m_path, 1, 0)
        SERIALISE_OBJECT(Runtime::ResourceTypeId, m_typeId, 1, 0)
        SERIALISE_PROPERTY(u64, m_id, 1, 2)
        );
}

namespace std
{
    template<>
    struct hash<Insight::Runtime::ResourceId>
    {
        size_t operator()(Insight::Runtime::ResourceId const& ID) const
        {
            return Insight::Algorithm::GetHash64(ID.GetPath());
        }
    };
}