#include "Resource/Resource.h"

#include "Resource/ResourceManager.h"
#include "Resource/Model.h"
#include "Resource/Mesh.h"
#include "Resource/Material.h"
#include "Resource/Texture2D.h"

#include "FileSystem/FileSystem.h"

#include "Core/Logger.h"

#include "Threading/TaskSystem.h"

#include "Algorithm/Vector.h"

#include <filesystem>

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
			case EResoruceStates::Not_Found:		return "Not Found";
			case EResoruceStates::Queued:			return "Queued";
			case EResoruceStates::Loaded:			return "Loaded";
			case EResoruceStates::Loading:		return "Loading";
			case EResoruceStates::Not_Loaded:		return "Not Loaded";
			case EResoruceStates::Failed_To_Load:	return "Failed To Load";
			case EResoruceStates::Unloaded:		return "Unloaded";
			case EResoruceStates::Unloading:		return "Unloading";
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

		IResource::IResource(std::string_view filePath)
		{
			m_resource_state = EResoruceStates::Not_Loaded;
			m_file_path = filePath;
		}

		IResource::~IResource()
		{ }

		IS_SERIALISABLE_CPP(IResource)

		std::string IResource::GetFilePath() const
		{
			//std::lock_guard lock(m_mutex);
			return m_file_path;
		}

		std::string IResource::GetFileName() const
		{
			//std::lock_guard lock(m_mutex);
			if (m_file_path.find_last_of('/') != std::string::npos)
			{
				return m_file_path.substr(m_file_path.find_last_of('/'), m_file_path.find('.') - m_file_path.find_last_of('/'));
			}
			return m_file_path;
		}

		EResoruceStates IResource::GetResourceState() const
		{
			//std::lock_guard lock(m_referenceLinksMutex);
			// If this resource is dependent on another then return that resource's current state.
			//auto iter = Algorithm::VectorFindIf(m_reference_links, [](ResourceReferenceLink const& link)
			//	{
			//		return link.GetReferenceLinkType() == ResourceReferenceLinkType::Dependent;
			//	});

			//if (iter != m_reference_links.end())
			//{
			//	return iter->GetLinkResource()->GetResourceState();
			//}
			return m_resource_state;
		}

		const ResourceReferenceLink* IResource::GetReferenceLink(u32 index) const
		{
			std::lock_guard lock(m_referenceLinksMutex);
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
			std::lock_guard lock(m_referenceLinksMutex);
			return Algorithm::VectorFindIf(m_reference_links, [](const ResourceReferenceLink& link)
				{
					return link.GetReferenceLinkType() == ResourceReferenceLinkType::Dependent;
				}) != m_reference_links.end();
		}

		bool IResource::IsDependentOnAnotherResource(IResource* resource) const
		{
			std::lock_guard lock(m_referenceLinksMutex);
			return Algorithm::VectorFindIf(m_reference_links, [resource](const ResourceReferenceLink& link)
				{
					return link.GetReferenceLinkType() == ResourceReferenceLinkType::Dependent && link.GetLinkResource() == resource;
				}) != m_reference_links.end();
		}

		bool IResource::IsDependentOwnerOnAnotherResource() const
		{
			std::lock_guard lock(m_referenceLinksMutex);
			return Algorithm::VectorFindIf(m_reference_links, [](const ResourceReferenceLink& link)
				{
					return link.GetReferenceLinkType() == ResourceReferenceLinkType::Dependent_Owner;
				}) != m_reference_links.end();
		}

		bool IResource::IsDependentOwnerOnAnotherResource(IResource* resource) const
		{
			std::lock_guard lock(m_referenceLinksMutex);
			return Algorithm::VectorFindIf(m_reference_links, [resource](const ResourceReferenceLink& link)
				{
					return link.GetReferenceLinkType() == ResourceReferenceLinkType::Dependent_Owner && link.GetLinkResource() == resource;
				}) != m_reference_links.end();
		}

		std::vector<ResourceReferenceLink> IResource::GetReferenceLinks() const
		{
			return m_reference_links;
		}

		ResourceTypeId IResource::GetResourceTypeId() const
		{
			FAIL_ASSERT_MSG("[Resource::GetResourceType] Resource is missing 'REGISTER_RESOURCE' marco.");
			return ResourceTypeId();
		}

		const char* IResource::GetResourceFileExtension() const
		{
			FAIL_ASSERT_MSG("[IResource::GetResourceFileExtension] Resource is missing 'REGISTER_RESOURCE' marco.");
			return nullptr;
		}

		const AssetInfo* IResource::GetAssetInfo() const
		{
			return m_assetInfo;
		}

		ResourceId IResource::GetResourceId() const
		{
			return m_resourceId;
		}

		void IResource::Print() const
		{
			std::lock_guard lock(m_mutex);
			IS_LOG_CORE_INFO("Resource:");
			IS_LOG_CORE_INFO("\tType: {}", GetResourceTypeId().GetTypeName());
			IS_LOG_CORE_INFO("\tSource path: {}", m_source_file_path);
			IS_LOG_CORE_INFO("\tFile path: {}", m_file_path);
			IS_LOG_CORE_INFO("\tState: {}", ERsourceStatesToString(m_resource_state));
			IS_LOG_CORE_INFO("\tStorage type: {}", ResourceStorageTypesToString(m_storage_type));
			IS_LOG_CORE_INFO("\n");
		}

		bool IResource::IsEngineFormat() const
		{
			std::string_view fileExtension = FileSystem::GetFileExtension(m_file_path);
			std::string_view extension = FileSystem::GetExtension(m_file_path);
			bool isFileExtension = fileExtension == GetResourceFileExtension();
			bool isExtension = extension == GetResourceFileExtension();
			return isFileExtension || isExtension;
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
			IResource* resource = ResourceManager::Instance().Load(ResourceId(file_path, type_id));

			if (resource)
			{
				std::lock_guard lock(m_referenceLinksMutex);
				std::lock_guard resourceLock(resource->m_referenceLinksMutex);
				resource->m_reference_links.push_back(ResourceReferenceLink(ResourceReferenceLinkType::Reference, resource, this));
				m_reference_links.push_back(ResourceReferenceLink(ResourceReferenceLinkType::Reference, this, resource));
			}

			if (resource
				&& resource->GetResourceState() != EResoruceStates::Loading)
			{
				IS_LOG_CORE_ERROR("[IResource::AddReferenceResource] Attempted load on resource '{0}' failed.", file_path);
			}
			return resource;
		}

		void IResource::AddReferenceResource(IResource* resource)
		{
			if (resource)
			{
				std::lock_guard lock(m_referenceLinksMutex);
				std::lock_guard resourceLock(resource->m_referenceLinksMutex);
				resource->m_reference_links.push_back(ResourceReferenceLink(ResourceReferenceLinkType::Reference, resource, this));
				m_reference_links.push_back(ResourceReferenceLink(ResourceReferenceLinkType::Reference, this, resource));
			}
		}

		void IResource::RemoveDependentResource(IResource* resource)
		{
			if (resource)
			{
				{
					// Remove the link from the other resource to this resource.
					std::lock_guard resourceLock(resource->m_referenceLinksMutex);
					Algorithm::VectorRemoveIf(resource->m_reference_links, [&](ResourceReferenceLink const& link)
						{
							return link.GetReferenceLinkType() == ResourceReferenceLinkType::Dependent 
								&& link.GetLinkResource() == this;
						});
				}
				{
					// Remove the link from this resource to the other resource.
					std::lock_guard lock(m_referenceLinksMutex);
					Algorithm::VectorRemoveIf(m_reference_links, [&](ResourceReferenceLink const& link)
						{
							return link.GetReferenceLinkType() == ResourceReferenceLinkType::Dependent_Owner
								&& link.GetLinkResource() == resource;
						});
				}
			}
		}

		void IResource::RemoveReferenceResource(IResource* resource)
		{
			if (resource)
			{
				{
					// Remove the link from the other resource to this resource.
					std::lock_guard resourceLock(resource->m_referenceLinksMutex);
					Algorithm::VectorRemoveIf(resource->m_reference_links, [&](ResourceReferenceLink const& link)
						{
							return link.GetReferenceLinkType() == ResourceReferenceLinkType::Reference
								&& link.GetLinkResource() == this;
						});
				}
				{
					// Remove the link from this resource to the other resource.
					std::lock_guard lock(m_referenceLinksMutex);
					Algorithm::VectorRemoveIf(m_reference_links, [&](ResourceReferenceLink const& link)
						{
							return link.GetReferenceLinkType() == ResourceReferenceLinkType::Reference
								&& link.GetLinkResource() == resource;
						});
				}
			}
		}

		ResourceId IResource::ConvertToEngineFormat()
		{
			FAIL_ASSERT_MSG("[IResource::ConvertToEngineFormat] Must be implemented.");
			return {};
		}

		IResource* IResource::AddDependentResource(const std::string& file_path, IResource* resource, ResourceStorageTypes storage_type, ResourceTypeId type_id)
		{
			if (resource == nullptr)
			{
				resource = ResourceManager::Instance().Load(ResourceId(file_path, type_id));
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