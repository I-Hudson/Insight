#include "Resource/ResourceTypeId.h"

#include "Core/Logger.h"

namespace Insight
{
	namespace Runtime
	{
		std::unordered_map<ResourceTypeId, ResourceTypeIdToResource::CreateFunc> ResourceTypeIdToResource::m_map;

		ResourceTypeId::ResourceTypeId(const char* type_name)
			: m_type_name(type_name)
			, m_hash(std::hash<ResourceTypeId>()(*this))
		{
		}

		std::string ResourceTypeId::GetTypeName() const
		{
			return m_type_name;
		}

		u64 ResourceTypeId::GetHash() const
		{
			return m_hash;
		}

		bool ResourceTypeId::operator==(ResourceTypeId const& other) const
		{
			return m_hash == other.m_hash;
		}

		bool ResourceTypeId::operator!=(ResourceTypeId const& other) const
		{
			return !(*this == other);
		}


		void ResourceTypeIdToResource::RegisterResource(ResourceTypeId type_id, CreateFunc func)
		{
			if (auto itr = m_map.find(type_id); itr != m_map.end())
			{
				IS_CORE_WARN("[ResourceTypeIdToResource::RegisterResource] Resource type is aleady registered '{}'.", type_id.GetTypeName());
				return;
			}
			m_map[type_id] = func;
		}

		IResource* ResourceTypeIdToResource::CreateResource(ResourceTypeId type_id)
		{
			if (auto itr = m_map.find(type_id); itr != m_map.end())
			{
				return itr->second();
			}
			IS_CORE_WARN("[ResourceTypeIdToResource::CreateResource] ");
			return nullptr;
		}
	}
}