#include "Resource/ResourceTypeId.h"
#include "Core/Logger.h"

namespace Insight
{
	namespace Runtime
	{
		std::unordered_map<ResourceTypeId, ResourceRegister::CreateFunc> ResourceRegister::m_map;
		std::unordered_map<std::string, ResourceTypeId> ResourceRegister::s_resourceExtensionToResourceTypeId;


		ResourceTypeId::ResourceTypeId()
		{ }

		ResourceTypeId::ResourceTypeId(const char* type_name, const char* extension)
			: m_type_name(type_name)
			, m_extension(extension)
		{
		}

		IS_SERIALISABLE_CPP(ResourceTypeId)

		std::string ResourceTypeId::GetTypeName() const
		{
			return m_type_name;
		}

		std::string ResourceTypeId::GetExtension() const
		{
			return m_extension;
		}

		ResourceTypeId::operator bool() const
		{
			return !m_type_name.empty();
		}

		bool ResourceTypeId::operator==(ResourceTypeId const& other) const
		{
			return m_type_name == other.m_type_name;
		}

		bool ResourceTypeId::operator!=(ResourceTypeId const& other) const
		{
			return !(*this == other);
		}

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
			IS_LOG_CORE_WARN("[ResourceTypeIdToResource::CreateResource] No resource with extension '{0}' registered.", fileExtension);
			return ResourceTypeId();
		}

		std::vector<ResourceTypeId> ResourceRegister::GetAllResourceTypeIds()
		{
			std::vector<ResourceTypeId> typeIds;
			for (const auto& [typeId, createFunc] : m_map)
			{
				typeIds.push_back(typeId);
			}
			return typeIds;
		}

		IResource* ResourceRegister::CreateResource(ResourceTypeId type_id, std::string_view filePath)
		{
			if (auto itr = m_map.find(type_id); itr != m_map.end())
			{
				return itr->second(filePath);
			}
			FAIL_ASSERT();
			IS_LOG_CORE_WARN("[ResourceTypeIdToResource::CreateResource] ");
			return nullptr;
		}

		void ResourceRegister::RegisterResourceAlreadyRegsiteredLog(const char* type)
		{
			IS_LOG_CORE_WARN("[ResourceTypeIdToResource::RegisterResource] Resource type is already registered '{}'.", type);
		}
	}
}