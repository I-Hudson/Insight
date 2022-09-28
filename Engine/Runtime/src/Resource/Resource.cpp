#include "Resource/Resource.h"

#include <filesystem>

namespace Insight
{
	namespace Runtime
	{
		std::string ERsourceStateToString(EResoruceState state)
		{
			return std::string();
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

		ResourceTypeId IResource::GetResourceTypeId() const
		{
			FAIL_ASSERT_MSG("[Resource::GetResourceType] Resource is missing 'REGISTER_RESOURCE' marco.");
			return ResourceTypeId("Unknown");
		}

		void IResource::Load(std::string file_path)
		{
			FAIL_ASSERT_MSG("[Resource::Load] Must be implemented.");
		}

		void IResource::UnLoad()
		{
			FAIL_ASSERT_MSG("[Resource::UnLoad] Must be implemented.");
		}


		ResourceManager::~ResourceManager()
		{
			UnloadAll();
			m_resources.clear();
		}

		Ptr<IResource> ResourceManager::Load(std::string file_path, ResourceTypeId type_id)
		{
			if (auto itr = m_resources.find(file_path); itr != m_resources.end())
			{
				if (itr->second->m_resource_state == EResoruceState::Not_Found)
				{
					IS_CORE_ERROR("[ResourceManager::Load] '{}' is not a valid file path. Is the file missing?", file_path);
					return nullptr;
				}
				else if (itr->second->m_resource_state == EResoruceState::Not_Loaded)
				{
					itr->second->Load(file_path);
				}
				++m_loaded_resource_count;
				return itr->second;
			}



			++m_loaded_resource_count;
			return Ptr<IResource>();
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
			resource->UnLoad();
			resource->m_resource_state = EResoruceState::Not_Loaded;
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