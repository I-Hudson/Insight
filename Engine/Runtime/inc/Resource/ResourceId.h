#pragma once

#include "Core/TypeAlias.h"

#include "Resource/ResourceTypeId.h"

#include <string>

namespace Insight
{
    namespace Runtime
    {
        class IS_RUNTIME ResourceId
        {
        public:
            ResourceId();
            ResourceId(const std::string& path, ResourceTypeId typeId);
            ResourceId(std::string&& path, ResourceTypeId typeId);
            ResourceId(const ResourceId& other);
            ResourceId(ResourceId&& other);
            ~ResourceId();

            const std::string& GetPath() const;
            const ResourceTypeId& GetTypeId() const;
            u64 GetId() const;

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

    //OBJECT_SERIALISER(Runtime::ResourceId, 1,
    //    SERIALISE_PROPERTY(std::string, m_path, 1, 0)
    //    SERIALISE_OBJECT(Runtime::ResourceTypeId, m_typeId, 1, 0)
    //    SERIALISE_PROPERTY(u64, m_id, 1, 0)
    //    );
}

namespace std
{
    template<>
    struct hash<Insight::Runtime::ResourceId>
    {
        size_t operator()(Insight::Runtime::ResourceId const& ID) const
        {
            return ID.GetId();
        }
    };
}