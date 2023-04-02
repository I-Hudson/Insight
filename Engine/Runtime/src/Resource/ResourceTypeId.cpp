#include "Resource/ResourceTypeId.h"

namespace Insight
{
	namespace Runtime
	{
		std::unordered_map<ResourceTypeId, ResourceRegister::CreateFunc> ResourceRegister::m_map;
		std::unordered_map<std::string, ResourceTypeId> ResourceRegister::s_resourceExtensionToResourceTypeId;


		ResourceTypeId::ResourceTypeId()
		{ }

		ResourceTypeId::ResourceTypeId(const char* type_name)
			: m_type_name(type_name)
			//, m_hash(std::hash<ResourceTypeId>()(*this))
		{
		}

		IS_SERIALISABLE_CPP(ResourceTypeId)

		std::string ResourceTypeId::GetTypeName() const
		{
			return m_type_name;
		}

		//u64 ResourceTypeId::GetHash() const
		//{
		//	return m_hash;
		//}

		ResourceTypeId::operator bool() const
		{
			return !m_type_name.empty();
		}

		bool ResourceTypeId::operator==(ResourceTypeId const& other) const
		{
			//return m_hash == other.m_hash;
			return m_type_name == other.m_type_name;
		}

		bool ResourceTypeId::operator!=(ResourceTypeId const& other) const
		{
			return !(*this == other);
		}


		//void ResourceRegister::RegisterResource(ResourceTypeId type_id, CreateFunc func)
		//{
		//	if (auto itr = m_map.find(type_id); itr != m_map.end())
		//	{
		//		IS_CORE_WARN("[ResourceTypeIdToResource::RegisterResource] Resource type is aleady registered '{}'.", type_id.GetTypeName());
		//		return;
		//	}
		//	m_map[type_id] = func;
		//}

		ResourceTypeId ResourceRegister::GetResourceTypeIdFromExtension(std::string_view fileExtension)
		{
			return GetResourceTypeIdFromExtension(std::string(fileExtension));
		}

		ResourceTypeId ResourceRegister::GetResourceTypeIdFromExtension(const std::string& fileExtension)
		{
			if (auto extensionItr = s_resourceExtensionToResourceTypeId.find(fileExtension);
				extensionItr != s_resourceExtensionToResourceTypeId.end())
			{
				return extensionItr->second;
			}
			IS_CORE_WARN("[ResourceTypeIdToResource::CreateResource] No resource with extension '{0}' registered.", fileExtension);
			return ResourceTypeId();
		}

		IResource* ResourceRegister::CreateResource(ResourceTypeId type_id)
		{
			if (auto itr = m_map.find(type_id); itr != m_map.end())
			{
				return itr->second();
			}
			FAIL_ASSERT();
			IS_CORE_WARN("[ResourceTypeIdToResource::CreateResource] ");
			return nullptr;
		}
	}
}