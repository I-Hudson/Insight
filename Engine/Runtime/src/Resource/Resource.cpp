#include "Resource/Resource.h"

#include "Resource/ResourceManager.h"
#include "Resource/Model.h"
#include "Resource/Mesh.h"
#include "Resource/Material.h"
#include "Resource/Texture2D.h"

#include "Core/Logger.h"

#include "Threading/TaskSystem.h"

#include <filesystem>
#include <ppltasks.h>

namespace Insight
{
	namespace Runtime
	{
#define RESOURCE_LOAD_THREAD

		const char* ResourceStorageTypesToString(ResourceStorageTypes storage_type)
		{
			switch (storage_type)
			{
			case Insight::Runtime::ResourceStorageTypes::Unknown:	return "Unknown";
			case Insight::Runtime::ResourceStorageTypes::Disk:		return "Disk";
			case Insight::Runtime::ResourceStorageTypes::Memory:	return "Memory";
			default:
				break;
			}
			FAIL_ASSERT();
			return "";
		}

		const char* ERsourceStatesToString(EResoruceStates state)
		{
			switch (state)
			{
			case Insight::Runtime::EResoruceStates::Not_Found:		return "Not Found";
			case Insight::Runtime::EResoruceStates::Loaded:			return "Loaded";
			case Insight::Runtime::EResoruceStates::Loading:			return "Loading";
			case Insight::Runtime::EResoruceStates::Not_Loaded:		return "Not Loaded";
			case Insight::Runtime::EResoruceStates::Failed_To_Load:	return "Failed To Load";
			case Insight::Runtime::EResoruceStates::Unloaded:		return "Unloaded";
			case Insight::Runtime::EResoruceStates::Unloading:		return "Unloading";
			default:
				break;
			}
			FAIL_ASSERT();
			return "";
		}


		//--------------------------------------------------------------------------
		// ResourceReferenceLink
		//--------------------------------------------------------------------------
		ResourceReferenceLink::ResourceReferenceLink(ResourceReferenceLinkType reference_type, IResource* this_resource, IResource* link_resource)
			: m_reference_link_type(reference_type)
			, m_this_resource(this_resource)
			, m_link_resource(link_resource)
		{ }


		//--------------------------------------------------------------------------
		// IResource
		//--------------------------------------------------------------------------
		IResource::IResource()
		{
			m_resource_state = EResoruceStates::Not_Loaded;
		}

		IResource::~IResource()
		{ }

		std::string IResource::GetFilePath() const
		{
			//std::lock_guard lock(m_mutex);
			return m_file_path;
		}

		std::string IResource::GetFileName() const
		{
			//std::lock_guard lock(m_mutex);
			return m_file_path.substr(m_file_path.find_last_of('/'), m_file_path.find('.') - m_file_path.find_last_of('/'));
		}

		EResoruceStates IResource::GetResourceState() const
		{
			return m_resource_state;
		}

		const ResourceReferenceLink* IResource::GetReferenceLink(u32 index) const
		{
			std::lock_guard lock(m_mutex);
			if (index < m_reference_links.size())
			{
				return &m_reference_links.at(index);
			}
			return nullptr;
		}

		ResourceStorageTypes IResource::GetResourceStorageType() const
		{
			std::lock_guard lock(m_mutex);
			return m_storage_type;
		}

		bool IResource::IsNotFound() const
		{
			return m_resource_state.load() == EResoruceStates::Not_Found;
		}

		bool IResource::IsLoaded() const
		{
			return m_resource_state.load() == EResoruceStates::Loaded;
		}

		bool IResource::IsNotLoaded() const
		{
			return m_resource_state.load() == EResoruceStates::Not_Loaded;
		}

		bool IResource::IsFailedToLoad() const
		{
			return m_resource_state.load() == EResoruceStates::Failed_To_Load;
		}

		bool IResource::IsUnloaded() const
		{
			return m_resource_state.load() == EResoruceStates::Unloaded;
		}

		bool IResource::IsDependentOnAnotherResource() const
		{
			std::lock_guard lock(m_mutex);
			return std::find_if(m_reference_links.begin(), m_reference_links.end(), [](const ResourceReferenceLink& link)
				{
					return link.GetReferenceLinkType() == ResourceReferenceLinkType::Dependent;
				}) != m_reference_links.end();
		}

		bool IResource::IsDependentOnAnotherResource(IResource* resource) const
		{
			std::lock_guard lock(m_mutex);
			return std::find_if(m_reference_links.begin(), m_reference_links.end(), [resource](const ResourceReferenceLink& link)
				{
					return link.GetReferenceLinkType() == ResourceReferenceLinkType::Dependent && link.GetLinkResource() == resource;
				}) != m_reference_links.end();
		}

		bool IResource::IsDependentOwnerOnAnotherResource() const
		{
			std::lock_guard lock(m_mutex);
			return std::find_if(m_reference_links.begin(), m_reference_links.end(), [](const ResourceReferenceLink& link)
				{
					return link.GetReferenceLinkType() == ResourceReferenceLinkType::Dependent_Owner;
				}) != m_reference_links.end();
		}

		bool IResource::IsDependentOwnerOnAnotherResource(IResource* resource) const
		{
			std::lock_guard lock(m_mutex);
			return std::find_if(m_reference_links.begin(), m_reference_links.end(), [resource](const ResourceReferenceLink& link)
				{
					return link.GetReferenceLinkType() == ResourceReferenceLinkType::Dependent_Owner && link.GetLinkResource() == resource;
				}) != m_reference_links.end();
		}

		ResourceTypeId IResource::GetResourceTypeId() const
		{
			FAIL_ASSERT_MSG("[Resource::GetResourceType] Resource is missing 'REGISTER_RESOURCE' marco.");
			return ResourceTypeId("Unknown");
		}

		ResourceId IResource::GetResourceId() const
		{
			return m_resourceId;
		}

		void IResource::Print() const
		{
			std::lock_guard lock(m_mutex);
			IS_CORE_INFO("Resource:");
			IS_CORE_INFO("\tType: {}", GetResourceTypeId().GetTypeName());
			IS_CORE_INFO("\tSource path: {}", m_source_file_path);
			IS_CORE_INFO("\tFile path: {}", m_file_path);
			IS_CORE_INFO("\tState: {}", ERsourceStatesToString(m_resource_state));
			IS_CORE_INFO("\tStorage type: {}", ResourceStorageTypesToString(m_storage_type));
			IS_CORE_INFO("\n");
		}

		IResource* IResource::AddDependentResourceFromDisk(const std::string& file_path, ResourceTypeId type_id)
		{
			return AddDependentResource(file_path, nullptr, ResourceStorageTypes::Disk, type_id);
		}

		IResource* IResource::AddDependentResource(IResource* resource)
		{
			return AddDependentResource("", resource, resource->GetResourceStorageType(), resource->GetResourceTypeId());
		}

		IResource* IResource::AddReferenceResource(const std::string& file_path, ResourceTypeId type_id)
		{
			IResource* resource = ResourceManager::Load(ResourceId(file_path, type_id));

			if (resource)
			{
				std::lock_guard lock(m_mutex);
				std::lock_guard resourceLock(resource->m_mutex);
				resource->m_reference_links.push_back(ResourceReferenceLink(ResourceReferenceLinkType::Reference, resource, this));
				m_reference_links.push_back(ResourceReferenceLink(ResourceReferenceLinkType::Reference, this, resource));
			}

			if (resource
				&& resource->GetResourceState() != EResoruceStates::Loading)
			{
				IS_CORE_ERROR("[IResource::AddReferenceResource] Attempted load on resource '{0}' failed.", file_path);
			}
			return resource;
		}

		void IResource::AddReferenceResource(IResource* resource)
		{
			if (resource)
			{
				std::lock_guard lock(m_mutex);
				std::lock_guard resourceLock(resource->m_mutex);
				resource->m_reference_links.push_back(ResourceReferenceLink(ResourceReferenceLinkType::Reference, resource, this));
				m_reference_links.push_back(ResourceReferenceLink(ResourceReferenceLinkType::Reference, this, resource));
			}
		}

		IResource* IResource::AddDependentResource(const std::string& file_path, IResource* resource, ResourceStorageTypes storage_type, ResourceTypeId type_id)
		{
			if (resource == nullptr)
			{
				resource = ResourceManager::Load(ResourceId(file_path, type_id));
			}

			if (resource)
			{
				std::lock_guard lock(m_mutex);
				std::lock_guard resourceLock(resource->m_mutex);
				resource->m_reference_links.push_back(ResourceReferenceLink(ResourceReferenceLinkType::Dependent, resource, this));
				m_reference_links.push_back(ResourceReferenceLink(ResourceReferenceLinkType::Dependent_Owner, this, resource));
			}
			return resource;
		}

		void IResource::Load()
		{
			FAIL_ASSERT_MSG("[Resource::Load] Must be implemented.");
		}

		void IResource::LoadFromMemory(const void* data, u64 size_in_bytes)
		{
			FAIL_ASSERT_MSG("[Resource::LoadFromMemory] Must be implemented.");
		}

		void IResource::UnLoad()
		{
			FAIL_ASSERT_MSG("[Resource::UnLoad] Must be implemented.");
		}

		void IResource::Save(const std::string& file_path)
		{
			FAIL_ASSERT_MSG("[Resource::Save] Must be implemented.");
		}

		void IResource::StartRequestTimer()
		{
			std::lock_guard lock(m_mutex);
			m_request_timer.Start();
		}

		void IResource::StopRequestTimer()
		{
			std::lock_guard lock(m_mutex);
			m_request_timer.Stop();
		}

		void IResource::StartLoadTimer()
		{
			std::lock_guard lock(m_mutex);
			m_load_timer.Start();
		}

		void IResource::StopLoadTimer()
		{
			std::lock_guard lock(m_mutex);
			m_load_timer.Stop();
		}

		void IResource::StartUnloadTimer()
		{
			std::lock_guard lock(m_mutex);
			m_unload_timer.Start();
		}

		void IResource::StopUnloadTimer()
		{
			std::lock_guard lock(m_mutex);
			m_unload_timer.Stop();
		}
	}
}