#pragma once

#include "Runtime/Defines.h"
#include "Resource/ResourceTypeId.h"

#include "Core/Singleton.h"
#include "Core/Memory.h"

#include <string>

namespace Insight
{
	namespace Runtime
	{
		class ResourceManager;

		enum class EResoruceState
		{
			Not_Found,

			Loaded,
			Loading,
			Not_Loaded,
			
			Failed_To_Load,
			Unloaded,
			Unloading,

		};
		std::string ERsourceStateToString(EResoruceState state);

		//TODO Resources will need reference counting.
		/// @brief Interface for any resource class. A resource is any item which can be saved/loaded from disk.
		class IS_RUNTIME IResource
		{
		public:
			IResource();
			/// @brief Always call unload when object is being destroyed.
			virtual ~IResource();

			std::string GetFilePath() const;
			std::string GetFileName() const;
			EResoruceState GetResourceState() const;

			bool IsNotFound() const;
			bool IsLoaded() const;
			bool IsNotLoaded() const;
			bool IsFailedToLoad() const;
			bool IsUnloaded() const;

			virtual ResourceTypeId GetResourceTypeId() const;

		private:
			/// @brief Handle loading the resource from disk.
			/// @param file_path 
			virtual void Load();
			/// @brief Handle unloading the resource from memory.
			virtual void UnLoad();

		protected:
			/// @brief Full file path.
			std::string m_file_path;
			/// @brief Current state of the resource.
			EResoruceState m_resource_state = EResoruceState::Not_Loaded;

			friend class ResourceManager;
		};

		class IS_RUNTIME ResourceManager : public Core::Singleton<ResourceManager>
		{
		public:
			ResourceManager();
			/// @brief Unload all resources currently loaded. (Use with caution).
			virtual ~ResourceManager() override;

			/// @brief Load resource at path 'file_path'. (This will first check if the resource is already loaded, if so then return the cached pointer.)
			/// @param file_path 
			/// @return Ptr<Resource>
			Ptr<IResource> Load(std::string file_path, ResourceTypeId type_id);

			/// @brief Unload a resource from memory.
			/// @param resouce 
			void Unload(Ptr<IResource> resouce);

			/// @brief Unload all currently loaded resources. (Use with caution).
			void UnloadAll();

			u32 GetLoadedResourcesCount() const;

		private:
			u32 m_loaded_resource_count = 0;
			std::unordered_map<std::string, UPtr<IResource>> m_resources;
		};
	}
}

#define REGISTER_RESOURCE(type_name) public: \
static Insight::Runtime::ResourceTypeId GetStaticResourceTypeId() { return Insight::Runtime::ResourceTypeId(#type_name); } \
Insight::Runtime::ResourceTypeId GetResourceTypeId() const override { return GetStaticResourceTypeId(); } \