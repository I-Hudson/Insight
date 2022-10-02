#pragma once

#include "Runtime/Defines.h"
#include "Resource/ResourceTypeId.h"

#include "Core/Singleton.h"
#include "Core/Memory.h"
#include "Core/Timer.h"

#include <string>
#include <atomic>

namespace Insight
{
	namespace Runtime
	{
		class IResource;
		class ResourceManager;

		enum class ResourceStorageTypes
		{
			Unknown,	/// @brief Resource storage location is unknown.
			Disk,		/// @brief Resource is storage on disk (This should always be the default).
			Memory,		/// @brief Resource is storage in memory only (This resource must be apart of another one).
		};
		CONSTEXPR const char* ResourceStorageTypesToString(ResourceStorageTypes storage_type);

		enum class EResoruceStates
		{
			Not_Found,		/// @brief Resource has not been found on disk.
									   
			Loaded,			/// @brief Resource has been loaded.
			Loading,		/// @brief Resource is being loaded.
			Not_Loaded,		/// @brief Resource has not been loaded yet.
									   
			Failed_To_Load,	/// @brief Resource failed to load.
			Unloaded,		/// @brief Resource has been unloaded.
			Unloading,		/// @brief Resource is been unloaded.

		};
		CONSTEXPR const char* ERsourceStatesToString(EResoruceStates state);

		enum class ResourceReferenceLinkType
		{
			Dependent,			/// @brief This resource depends on another. (Mesh from a Model)
			Dependent_Owner,	/// @brief This resource "owns" other resources. (When this is unloaded the others will be too)
			Reference,			/// @brief This resource just references another. If the original resource is unloaded this ones isn't. (Texture from a model).
		};

		/**
			@brief Define links between resrouces.
			This could be if a resource depends on another one (Texture depends on a Model resource)
		*/
		class ResourceReferenceLink
		{
		public:
			ResourceReferenceLink(ResourceReferenceLinkType reference_type, IResource* this_resource, IResource* link_resource);

			ResourceReferenceLinkType GetReferenceLinkType() const { return m_reference_link_type; }
			IResource* GetResource()                         const { return m_this_resource; }
			IResource* GetLinkResource()                     const { return m_link_resource; }

		private:
			ResourceReferenceLinkType m_reference_link_type;
			/// @brief Point to the resource this reference link is a part of.
			IResource* m_this_resource;
			/// @brief Point to the resource which this link is tied to.
			IResource* m_link_resource;
		};

		/// @brief Interface for any resource class. A resource is any item which can be saved/loaded from disk.
		class IS_RUNTIME IResource
		{
		public:
			IResource();
			virtual ~IResource();

			/// @brief Return thr full file path (From drive letter C:\\).
			std::string GetFilePath() const;
			/// @brief Return the file name.
			std::string GetFileName() const;
			/// @brief Return the current state of this resource.
			EResoruceStates GetResourceState() const;

			bool IsNotFound() const;
			bool IsLoaded() const;
			bool IsNotLoaded() const;
			bool IsFailedToLoad() const;
			bool IsUnloaded() const;

			/// @brief Print information about this resource to the output log.
			void Print() const;

			virtual ResourceTypeId GetResourceTypeId() const;

		protected:
			/// @brief Add a new resource which depends upon this resource. (Texture could depend upon the Model loaded).
			/// @param file_path 
			/// @param type_id 
			/// @return IResource*
			IResource* AddDependentResourceFromDisk(std::string file_path, ResourceTypeId type_id);

			/// @brief Add a new resource which depends upon this resource. But is stored in memory only. This could be a mesh which is a part of 
			/// a model. The mesh it self is not stored on disk as it self but as part of the model file.
			/// @param file_path 
			/// @param type_id 
			/// @return IResource*
			IResource* AddDependentResourceFromMemory(const void* data, u64 data_size_in_bytes, ResourceTypeId type_id);

			/// @brief 
			/// @param resource 
			/// @param storage_type 
			void AddDependentResrouce(IResource* resource, std::string file_path, ResourceStorageTypes storage_type);

			/// @brief Add a reference resource (this does will not own the resource) to this one. Reference resource 
			/// can only be loaded from disk as there are non owning.
			/// @param file_path 
			/// @param type_id
			/// @return IResource*
			IResource* AddReferenceResource(std::string file_path, ResourceTypeId type_id);

		private:
			IResource* AddDependentResource(std::string file_path, const void* data, const u64& data_size_in_bytes, ResourceStorageTypes storage_type, ResourceTypeId type_id);
			
			/// @brief Handle loading the resource from disk.
			/// @param file_path 
			virtual void Load();

			/// @brief Handle loading a resource from memory. This is called for a resource which is a "sub resource" of another one
			/// and exists inside another resource's disk file.
			/// @param file_path 
			virtual void LoadFromMemory(const void* data, u64 size_in_bytes);

			/// @brief Handle unloading the resource from memory.
			virtual void UnLoad();

		protected:
			/// @brief On disk file path. (In most cases this will be the same as 'm_file_path')
			std::string m_source_file_path;
			/// @brief Full file path.
			std::string m_file_path;
			/// @brief The current state of the resource.
			std::atomic<EResoruceStates> m_resource_state = EResoruceStates::Not_Loaded;
			/// @brief Define the location on the resource. Most should be ResourceStorageTypes::Disk.
			ResourceStorageTypes m_storage_type = ResourceStorageTypes::Unknown;
			/// @brief Vector of all the reference links this resource has to other ones.
			std::vector<ResourceReferenceLink> m_reference_links;

			/// @brief Timer for when the resource has been requested to end of life.
			Core::Timer m_request_timer;
			/// @brief Timer for the latest time the resource was loaded.
			Core::Timer m_load_timer;
			/// @brief Timer for the latest time the resource was unloaded.
			Core::Timer m_unload_timer;

			friend class ResourceManager;
		};

		class IS_RUNTIME ResourceManager : public Core::Singleton<ResourceManager>
		{
		public:
			ResourceManager();
			/// @brief Unload all resources currently loaded. (Use with caution. Should really only be called when the app is exiting).
			virtual ~ResourceManager() override;

			/// @brief Load resource at path 'file_path'. (This will first check if the resource is already loaded, if so then return the cached pointer.)
			/// @param file_path 
			/// @return Ptr<Resource>
			IResource* Load(std::string file_path, ResourceTypeId type_id);

			/// @brief Unload a resource from memory.
			/// @param resouce 
			void Unload(IResource* resouce);

			/// @brief Unload all currently loaded resources. (Use with caution).
			void UnloadAll();

			u32 GetLoadedResourcesCount() const;

			/// @brief Check if a resource exists with a given file path.
			/// @return bool
			bool HasResource(std::string_view file_path) const;

			/// @brief Export all the time stats for resource loading to a file.
			/// @param file_path 
			void ExportStatsToFile(std::string file_path);

			/// @brief Print current resources info to the output log.
			void Print();

		private:
			/// @brief Handle loading resources from disk and memory.
			IResource* Load(std::string file_path, const void* data, u64 data_size_in_bytes, ResourceStorageTypes storage_type, ResourceTypeId type_id);

			/// @brief Add an existing resource (resoruce generated not form loading). This should be use with caution.
			/// @param resource
			/// @param file_path
			void AddExistingResource(IResource* resource, std::string file_path);

		private:
			/// @brief Handle general data access;
			mutable std::mutex m_lock;
			/// @brief Load count of resources (from disk only).
			u32 m_loaded_resource_count = 0;
			std::unordered_map<std::string, UPtr<IResource>> m_resources;

			friend class IResource;
		};
	}
}

#define REGISTER_RESOURCE(type_name) public: \
static Insight::Runtime::ResourceTypeId GetStaticResourceTypeId() { return Insight::Runtime::ResourceTypeId(#type_name); } \
Insight::Runtime::ResourceTypeId GetResourceTypeId() const override { return GetStaticResourceTypeId(); } \