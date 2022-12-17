#include "Resource/ResourceId.h"

#include "Algorithm/Hash.h"
#include "FileSystem/FileSystem.h"

namespace Insight
{
    namespace Runtime
    {
        ResourceId::ResourceId()
            : m_typeId("")
        { }

        ResourceId::ResourceId(const std::string& path, ResourceTypeId typeId)
            : m_path(FileSystem::FileSystem::GetAbsolutePath(path))
            , m_typeId(std::move(typeId))
        {
            m_id = Algorithm::GetHash64(m_path);
        }

        ResourceId::ResourceId(std::string&& path, ResourceTypeId typeId)
            : m_path(FileSystem::FileSystem::GetAbsolutePath(path))
            , m_typeId(std::move(typeId))
        { 
            m_id = Algorithm::GetHash64(m_path);
        }

        ResourceId::ResourceId(const ResourceId& other)
            : m_path(other.m_path)
            , m_typeId(other.m_typeId)
            , m_id(other.m_id)
        { }

        ResourceId::ResourceId(ResourceId&& other)
            : m_path(other.m_path)
            , m_typeId(other.m_typeId)
            , m_id(other.m_id)
        {
            other.m_path = "";
            other.m_typeId = "";
            other.m_id = 0;
        }

        ResourceId::~ResourceId()
        { }

        const std::string& ResourceId::GetPath() const
        {
            return m_path;
        }

        const ResourceTypeId& ResourceId::GetTypeId() const
        {
            return m_typeId;
        }

        u64 ResourceId::GetId() const
        {
            return m_id;
        }

        bool ResourceId::operator==(ResourceId const& other) const
        {
            return m_path == other.m_path
                && m_typeId == other.m_typeId
                && m_id == other.m_id;
        }

        bool ResourceId::operator!=(ResourceId const& other) const
        {
            return !(*this == other);
        }

        ResourceId& ResourceId::operator=(ResourceId const& other)
        {
            m_path = other.m_path;
            m_typeId = other.m_typeId;
            m_id = other.m_id;
            return *this;
        }

        ResourceId& ResourceId::operator=(ResourceId&& other)
        {
            m_path = other.m_path;
            m_typeId = other.m_typeId;
            m_id = other.m_id;

            other.m_path = "";
            other.m_typeId = "";
            other.m_id = 0;
            return *this;
        }
    }
}