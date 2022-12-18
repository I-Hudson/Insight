#pragma once

#include "Core/Defines.h"
#include "Runtime/Defines.h"
#include "Resource/ResourceId.h"

#include "Core/Delegate.h"
#include "Core/Memory.h"
#include "Core/Singleton.h"
#include "Core/Timer.h"

#include <string>
#include <atomic>
#include <shared_mutex>

namespace Insight
{
	namespace Runtime
	{
		class IResource;
		class ResourceManager;
		class ResourceManagerExt;
		class ResourceDatabase;

		enum class ResourceStorageTypes
		{
			Unknown,	/// @brief Resource storage location is unknown.
			Disk,		/// @brief Resource is storage on disk (This should always be the default).
			Memory,		/// @brief Resource is storage in memory only (This resource must be a part of a resource which has been loaded from disk).
		};
		IS_RUNTIME const char* ResourceStorageTypesToString(ResourceStorageTypes storage_type);

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
		IS_RUNTIME const char* ERsourceStatesToString(EResoruceStates state);

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

			THREAD_SAFE;

			/// @brief Return thr full file path (From drive letter C:\\).
			std::string GetFilePath() const;
			/// @brief Return the file name.
			std::string GetFileName() const;
			/// @brief Return the current state of this resource.
			EResoruceStates GetResourceState() const;

			const ResourceReferenceLink* GetReferenceLink(u32 index) const;
			ResourceStorageTypes GetResourceStorageType() const;

			/// @brief Check if this resource is dependent on another resource (owned).
			/// @return bool 
			bool IsDependentOnAnotherResource() const;
			bool IsDependentOnAnotherResource(IResource* resource) const;
			bool IsDependentOwnerOnAnotherResource() const;
			bool IsDependentOwnerOnAnotherResource(IResource* resource) const;

			bool IsNotFound() const;
			bool IsLoaded() const;
			bool IsNotLoaded() const;
			bool IsFailedToLoad() const;
			bool IsUnloaded() const;

			/// @brief Print information about this resource to the output log.
			void Print() const;

			virtual ResourceTypeId GetResourceTypeId() const;
			ResourceId GetResourceId() const;

			Core::Timer GetLoadTimer() const { return m_load_timer; }

			Core::Delegate<IResource*> OnLoaded;
			Core::Delegate<IResource*> OnUnloaded;

		protected:
			/// @brief Add a new resource which depends upon this resource. (Texture could depend upon the Model loaded). 
			/// This will load the Resource.
			/// @param file_path 
			/// @param type_id 
			/// @return IResource*
			IResource* AddDependentResourceFromDisk(const std::string& file_path, ResourceTypeId type_id);

			/// @brief Add a new resource which depends upon this resource. This expects the Resource to already exists
			/// be that from being loaded or created internally as a memory only Resource.
			/// @param file_path 
			/// @param type_id 
			/// @return IResource*
			IResource* AddDependentResource(IResource* resource);

			/// @brief Add a reference resource (this does will not own the resource) to this one. Reference resource 
			/// can only be loaded from disk as there are non owning.
			/// @param file_path 
			/// @param type_id
			/// @return IResource*
			IResource* AddReferenceResource(const std::string& file_path, ResourceTypeId type_id);
			/// @brief 
			/// @param resource
			/// @param file_path
			/// @param storage_type
			void AddReferenceResource(IResource* resource);

		private:
			IResource* AddDependentResource(const std::string& file_path, IResource* resource, ResourceStorageTypes storage_type, ResourceTypeId type_id);
			
			/// @brief Handle loading the resource from disk.
			/// @param file_path 
			virtual void Load();

			/// @brief Handle loading a resource from memory. This is called for a resource which is a "sub resource" of another one
			/// and exists inside another resource's disk file.
			/// @param file_path 
			virtual void LoadFromMemory(const void* data, u64 size_in_bytes);

			/// @brief Handle unloading the resource from memory.
			virtual void UnLoad();

			/// @brief Save resrouce to disk.
			virtual void Save(const std::string& file_path);

			void StartRequestTimer();
			void StopRequestTimer();

			void StartLoadTimer();
			void StopLoadTimer();

			void StartUnloadTimer();
			void StopUnloadTimer();

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

			ResourceId m_resourceId;

			/// @brief Timer for when the resource has been requested to end of life.
			Core::Timer m_request_timer;
			/// @brief Timer for the latest time the resource was loaded.
			Core::Timer m_load_timer;
			/// @brief Timer for the latest time the resource was unloaded.
			Core::Timer m_unload_timer;

			mutable std::mutex m_mutex;

			friend class ResourceManager;
			friend class ResourceManagerExt; // Temp until old ResourceManager is removed.
			friend class ResourceDatabase;
		};
	}
}

#define REGISTER_RESOURCE(type_name) public: \
static Insight::Runtime::ResourceTypeId GetStaticResourceTypeId() { return Insight::Runtime::ResourceTypeId(#type_name); } \
Insight::Runtime::ResourceTypeId GetResourceTypeId() const override { return GetStaticResourceTypeId(); } \
constexpr const char* GetResourceFileExtension() { return STRINGIZE(PPCAT(.IS, type_name)); }