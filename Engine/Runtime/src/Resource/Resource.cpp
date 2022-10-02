#include "Resource/Resource.h"

#include "Resource/Model.h"
#include "Resource/Mesh.h"

#include <filesystem>

namespace Insight
{
	namespace Runtime
	{
		CONSTEXPR const char* ResourceStorageTypesToString(ResourceStorageTypes storage_type)
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

		CONSTEXPR const char* ERsourceStatesToString(EResoruceStates state)
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
			return m_file_path;
		}

		std::string IResource::GetFileName() const
		{
			return m_file_path.substr(m_file_path.find_last_of('/'), m_file_path.find('.') - m_file_path.find_last_of('/'));
		}

		EResoruceStates IResource::GetResourceState() const
		{
			return m_resource_state;
		}

		bool IResource::IsNotFound() const
		{
			return m_resource_state == EResoruceStates::Not_Found;
		}

		bool IResource::IsLoaded() const
		{
			return m_resource_state == EResoruceStates::Loaded;
		}

		bool IResource::IsNotLoaded() const
		{
			return m_resource_state == EResoruceStates::Not_Loaded;
		}

		bool IResource::IsFailedToLoad() const
		{
			return m_resource_state == EResoruceStates::Failed_To_Load;
		}

		bool IResource::IsUnloaded() const
		{
			return m_resource_state == EResoruceStates::Unloaded;
		}

		ResourceTypeId IResource::GetResourceTypeId() const
		{
			FAIL_ASSERT_MSG("[Resource::GetResourceType] Resource is missing 'REGISTER_RESOURCE' marco.");
			return ResourceTypeId("Unknown");
		}

		void IResource::Print() const
		{
			IS_CORE_INFO("Resource:");
			IS_CORE_INFO("\tType: {}", GetResourceTypeId().GetTypeName());
			IS_CORE_INFO("\tSource path: {}", m_source_file_path);
			IS_CORE_INFO("\tFile path: {}", m_file_path);
			IS_CORE_INFO("\tState: {}", ERsourceStatesToString(m_resource_state));
			IS_CORE_INFO("\tStorage type: {}", ResourceStorageTypesToString(m_storage_type));
		}

		IResource* IResource::AddDependentResourceFromDisk(std::string file_path, ResourceTypeId type_id)
		{
			return AddDependentResource(file_path, nullptr, 0, ResourceStorageTypes::Disk, type_id);
		}

		IResource* IResource::AddDependentResourceFromMemory(const void* data, u64 data_size_in_bytes, ResourceTypeId type_id)
		{
			return AddDependentResource("", data, data_size_in_bytes, ResourceStorageTypes::Memory, type_id);
		}

		void IResource::AddDependentResrouce(IResource* resource, std::string file_path, ResourceStorageTypes storage_type)
		{
			if (resource)
			{
				ResourceManager::Instance().AddExistingResource(resource, file_path);
				resource->m_storage_type = storage_type;
				resource->m_reference_links.push_back(ResourceReferenceLink(ResourceReferenceLinkType::Dependent, resource, this));
				m_reference_links.push_back(ResourceReferenceLink(ResourceReferenceLinkType::Dependent_Owner, this, resource));
			}
		}

		IResource* IResource::AddReferenceResource(std::string file_path, ResourceTypeId type_id)
		{
			IResource* resource = ResourceManager::Instance().Load(file_path, nullptr, 0, ResourceStorageTypes::Disk, type_id);
			if (resource)
			{
				resource->m_reference_links.push_back(ResourceReferenceLink(ResourceReferenceLinkType::Dependent, resource, this));
				m_reference_links.push_back(ResourceReferenceLink(ResourceReferenceLinkType::Dependent_Owner, this, resource));
			}
			return resource;
		}

		IResource* IResource::AddDependentResource(std::string file_path, const void* data, const u64& data_size_in_bytes, ResourceStorageTypes storage_type, ResourceTypeId type_id)
		{
			IResource* resource = ResourceManager::Instance().Load(file_path, data, data_size_in_bytes, storage_type, type_id);
			if (resource)
			{
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


		//--------------------------------------------------------------------------
		// ResourceManager
		//--------------------------------------------------------------------------
		ResourceManager::ResourceManager()
		{
			ResourceTypeIdToResource::RegisterResource(Model::GetStaticResourceTypeId(), []() { return NewTracked(Model); });
			ResourceTypeIdToResource::RegisterResource(Mesh::GetStaticResourceTypeId(), []() { return NewTracked(Mesh); });
		}

		ResourceManager::~ResourceManager()
		{
			UnloadAll();
			m_resources.clear();
		}

		IResource* ResourceManager::Load(std::string file_path, ResourceTypeId type_id)
		{
			return Load(file_path, nullptr, 0, ResourceStorageTypes::Disk, type_id);
		}

		void ResourceManager::Unload(IResource* resource)
		{
			if (!resource)
			{
				IS_CORE_ERROR("[ResourceManager::UnloadResource] The resource is not valid (null). The resource must be a valid pointer to a IResource.");
				return;
			}

			if (resource->GetResourceState() != EResoruceStates::Loaded)
			{
				IS_CORE_WARN("[ResourceManager::Unload] 'resource' current state is '{0}'. Resource must be loaded to be unloaded."
					, ERsourceStatesToString(resource->GetResourceState()));
			}

			// Unload the resource,
			// Remove the resoruce from the loaded map,
			resource->m_resource_state = EResoruceStates::Unloading;
			resource->m_unload_timer.Start();
			resource->UnLoad();
			resource->m_unload_timer.Stop();
			resource->m_resource_state = EResoruceStates::Unloaded;
			--m_loaded_resource_count;
		}

		void ResourceManager::UnloadAll()
		{
			std::lock_guard lock(m_lock);
			for (auto& pair : m_resources)
			{
				Unload(pair.second.Get());
			}
		}

		u32 ResourceManager::GetLoadedResourcesCount() const
		{
			return m_loaded_resource_count;
		}

		bool ResourceManager::HasResource(std::string_view file_path) const
		{
			std::lock_guard lock(m_lock);
			return m_resources.find(file_path.data()) != m_resources.end();
		}

		void ResourceManager::ExportStatsToFile(std::string file_path)
		{
			std::lock_guard lock(m_lock);
		}

		void ResourceManager::Print()
		{
			std::lock_guard lock(m_lock);
			for (const auto& pair : m_resources)
			{
				const IResource* resource = pair.second.Get();
				resource->Print();
			}
		}

		IResource* ResourceManager::Load(std::string file_path, const void* data, u64 data_size_in_bytes, ResourceStorageTypes storage_type, ResourceTypeId type_id)
		{
			std::filesystem::path abs_file_system_path = std::filesystem::absolute(file_path);
			std::string abs_file_path = abs_file_system_path.u8string();
			IResource* resource = nullptr;

			{
				std::lock_guard lock(m_lock);
				if (auto itr = m_resources.find(abs_file_path); itr == m_resources.end())
				{
					IResource* resource = ResourceTypeIdToResource::CreateResource(type_id);
					if (!resource)
					{
						IS_CORE_WARN("[ResourceManager::Load] Unable to create resource for id '{}'.", type_id.GetTypeName());
						return nullptr;
					}
					resource->m_file_path = abs_file_path;
					resource->m_resource_state = EResoruceStates::Not_Loaded;
					resource->m_storage_type = storage_type;
					resource->m_request_timer.Start();
					m_resources[abs_file_path] = resource;
				}

				if (auto itr = m_resources.find(abs_file_path); itr != m_resources.end())
				{
					resource = itr->second.Get();
				}
			}

			if (resource)
			{
				if (resource->IsLoaded())
				{
					// Item is already loaded, just return it.
					return resource;
				}

				if (resource->IsNotLoaded() || resource->IsUnloaded())
				{
					if (resource->m_storage_type == ResourceStorageTypes::Disk)
					{
						if (!std::filesystem::exists(abs_file_path))
						{
							// File does not exists. Set the resource state and return nullptr.
							resource->m_resource_state = EResoruceStates::Not_Found;
							return nullptr;
						}
						else
						{
							resource->m_resource_state = EResoruceStates::Loading;
							// Try and load the resource as it exists.
							resource->m_load_timer.Start();
							resource->Load();
							resource->m_load_timer.Stop();
						}
					}
					else
					{
						FAIL_ASSERT_MSG("[ResourceManager::Load] Maybe this should be done. Maybe when an resrouce is being loaded from disk it should handle loading memory resources.");
						resource->m_load_timer.Start();
						resource->LoadFromMemory(data, data_size_in_bytes);
						resource->m_load_timer.Stop();
					}
				}

				if (resource->IsLoaded())
				{
					// Resource loaded successfully.
					++m_loaded_resource_count;
				}
			}
			// Something went wrong at somepoint.
			return resource;
		}

		void ResourceManager::AddExistingResource(IResource* resource, std::string file_path)
		{

			std::filesystem::path abs_file_system_path = std::filesystem::absolute(file_path);
			std::string abs_file_path = abs_file_system_path.u8string();

			{
				std::lock_guard lock(m_lock);
				if (auto itr = m_resources.find(abs_file_path); itr != m_resources.end())
				{
					IS_CORE_ERROR("[ResourceManager::AddResource] There is already a resource with the file path of '{}'.", file_path);
					return;
				}
				m_resources[abs_file_path] = resource;
			}
			resource->m_resource_state = EResoruceStates::Loaded;
		}
	}
}