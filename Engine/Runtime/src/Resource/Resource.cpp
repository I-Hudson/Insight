#include "Resource/Resource.h"

#include "Resource/Model.h"

#include <filesystem>

namespace Insight
{
	namespace Runtime
	{
		std::string ERsourceStateToString(EResoruceState state)
		{
			switch (state)
			{
			case Insight::Runtime::EResoruceState::Not_Found:		return "Not Found";
			case Insight::Runtime::EResoruceState::Loaded:			return "Loaded";
			case Insight::Runtime::EResoruceState::Loading:			return "Loading";
			case Insight::Runtime::EResoruceState::Not_Loaded:		return "Not Loaded";
			case Insight::Runtime::EResoruceState::Failed_To_Load:	return "Failed To Load";
			case Insight::Runtime::EResoruceState::Unloaded:		return "Unloaded";
			case Insight::Runtime::EResoruceState::Unloading:		return "Unloading";
			default:
				break;
			}
			FAIL_ASSERT();
			return "";
		}

		IResource::IResource()
		{
			m_resource_state = EResoruceState::Not_Loaded;
		}

		IResource::~IResource()
		{
			UnLoad();
		}

		std::string IResource::GetFilePath() const
		{
			return m_file_path;
		}

		std::string IResource::GetFileName() const
		{
			return m_file_path.substr(m_file_path.find_last_of('/'), m_file_path.find('.') - m_file_path.find_last_of('/'));
		}

		EResoruceState IResource::GetResourceState() const
		{
			return m_resource_state;
		}

		bool IResource::IsNotFound() const
		{
			return m_resource_state == EResoruceState::Not_Found;
		}

		bool IResource::IsLoaded() const
		{
			return m_resource_state == EResoruceState::Loaded;
		}

		bool IResource::IsNotLoaded() const
		{
			return m_resource_state == EResoruceState::Not_Loaded;
		}

		bool IResource::IsFailedToLoad() const
		{
			return m_resource_state == EResoruceState::Failed_To_Load;
		}

		bool IResource::IsUnloaded() const
		{
			return m_resource_state == EResoruceState::Unloaded;
		}

		ResourceTypeId IResource::GetResourceTypeId() const
		{
			FAIL_ASSERT_MSG("[Resource::GetResourceType] Resource is missing 'REGISTER_RESOURCE' marco.");
			return ResourceTypeId("Unknown");
		}

		void IResource::Load()
		{
			FAIL_ASSERT_MSG("[Resource::Load] Must be implemented.");
		}

		void IResource::UnLoad()
		{
			FAIL_ASSERT_MSG("[Resource::UnLoad] Must be implemented.");
		}


		ResourceManager::ResourceManager()
		{
			ResourceTypeIdToResource::RegisterResource(Model::GetStaticResourceTypeId(), []() { return NewTracked(Model); });
		}

		ResourceManager::~ResourceManager()
		{
			UnloadAll();
			m_resources.clear();
		}

		Ptr<IResource> ResourceManager::Load(std::string file_path, ResourceTypeId type_id)
		{
			std::filesystem::path abs_file_system_path = std::filesystem::absolute(file_path);
			std::string abs_file_path = abs_file_system_path.u8string();

			if (auto itr = m_resources.find(abs_file_path); itr == m_resources.end())
			{
				IResource* resource = ResourceTypeIdToResource::CreateResource(type_id);
				if (!resource)
				{
					IS_CORE_WARN("[ResourceManager::Load] Unable to create resource for id '{}'.", type_id.GetTypeName());
					return nullptr;
				}
				resource->m_file_path = abs_file_path;
				resource->m_resource_state = EResoruceState::Not_Loaded;
				m_resources[abs_file_path] = std::move(UPtr(resource));
			}

			if (auto itr = m_resources.find(abs_file_path); itr != m_resources.end())
			{
				if (itr->second->IsLoaded())
				{
					// Item is already loaded, just return it.
					return itr->second;
				}

				if (itr->second->IsNotLoaded() || itr->second->IsUnloaded())
				{
					if (!std::filesystem::exists(abs_file_path))
					{
						// File does not exists. Set the resource state and return nullptr.
						itr->second->m_resource_state = EResoruceState::Not_Found;
						return nullptr;
					}
					else
					{
						itr->second->m_resource_state = EResoruceState::Loading;
						// Try and load the resource as it exists.
						itr->second->Load();
					}
				}

				if (itr->second->IsLoaded())
				{
					// Resource loaded successfully. Return it.
					++m_loaded_resource_count;
					return itr->second;
				}
			}
			// Something went wrong at somepoint.
			return nullptr;
		}

		void ResourceManager::Unload(Ptr<IResource> resource)
		{
			if (!resource)
			{
				IS_CORE_ERROR("[ResourceManager::Unload] 'resource' is not valid (null). 'resource' must be a valid pointer to a resource.");
				return;
			}

			if (resource->GetResourceState() != EResoruceState::Loaded)
			{
				IS_CORE_WARN("[ResourceManager::Unload] 'resource' current state is '{0}'. Resource must be loaded to be unloaded."
					, ERsourceStateToString(resource->GetResourceState()));
			}

			// Unload the resource,
			// Remove the resoruce from the loaded map,
			resource->m_resource_state = EResoruceState::Unloading;
			resource->UnLoad();
			resource->m_resource_state = EResoruceState::Unloaded;
			--m_loaded_resource_count;
		}

		void ResourceManager::UnloadAll()
		{
			for (auto& pair : m_resources)
			{
				Unload(pair.second);
			}
		}

		u32 ResourceManager::GetLoadedResourcesCount() const
		{
			return m_loaded_resource_count;
		}
	}
}