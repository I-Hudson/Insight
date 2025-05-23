#pragma once

#include "Core/TypeAlias.h"
#include "Graphics/Enums.h"
#include "Graphics/Defines.h"

#include "Graphics/RHI/RHI_ResourceRenderTracker.h"

#include <mutex>
#include <type_traits>
#include <queue>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <string>

namespace Insight
{
	namespace Graphics
	{
		struct RHI_UploadQueueRequestInternal;
		class RHI_UploadQueue;

		class IS_GRAPHICS RHI_Resource
		{
		public:
			virtual ~RHI_Resource() { }

			virtual void Release() = 0;
			virtual bool ValidResource() = 0;
			virtual void SetName(std::string name) = 0;

			DeviceUploadStatus GetUploadStatus() const { return m_uploadStatus; }

		public:
			std::string m_name;

		protected:
			DeviceUploadStatus m_uploadStatus = DeviceUploadStatus::NotUploaded;
			mutable std::mutex m_mutex;

			friend struct RHI_UploadQueueRequestInternal;
			friend class RHI_UploadQueue;
		};

		//void FreeResourceFromResourceManager(RHI_Resource* resource);

		template<typename T>
		class RHI_ResourceManager
		{
			THREAD_SAFE;
			static_assert(std::is_base_of_v<RHI_Resource, T>);

		public:
			using Type = T;
			using TypePtr = Type*;

		public:
			RHI_ResourceManager()
			{
				static_assert(!std::is_pointer_v<T>, "[RHI_ResourceManager] 'T' must not be a pointer.");
				static_assert(std::is_base_of_v<RHI_Resource, T>, "[RHI_ResourceManager] 'T' must derived from RHI_Resource.");
			}

			~RHI_ResourceManager()
			{
			}

			TypePtr CreateResource()
			{
				std::lock_guard lock(m_lock);
				TypePtr object = Type::New();
				m_objects.insert(object);
				return object;
			}

			void FreeResource(TypePtr object)
			{
				//ASSERT(Platform::IsMainThread());
				std::lock_guard lock(m_lock);
				auto itr = m_objects.find(object);
				if (itr != m_objects.end())
				{
					RHI_Resource* resource = static_cast<RHI_Resource*>(object);
					if (!RHI_ResourceRenderTracker::Instance().IsResourceInUse(resource))
					{
						resource->Release();
						Delete(resource);
					}
					else
					{
						RHI_ResourceRenderTracker::Instance().AddDeferedRelase([resource]()
							{
								RHI_Resource* res = resource;
								res->Release();
								Delete(res);
							});
					}
					m_objects.erase(itr);
				}
			}

			bool HasResource(TypePtr object) const
			{
				std::lock_guard lock(m_lock);
				return m_objects.find(object) != m_objects.end();
			}

			void ReleaseAll()
			{
				std::lock_guard lock(m_lock);
				for (auto obj : m_objects)
				{
					if (obj)
					{
						obj->Release();
						Delete(obj);
					}
				}
				m_objects.clear();
			}

			int GetSize() const { return static_cast<int>(m_objects.size()); }
			bool IsEmpty() const { return m_objects.size() == 0ull; }

			void Lock() const { m_lock.lock(); }
			void Unlock() const { m_lock.unlock(); }
			const std::unordered_set<TypePtr>& GetData() const { return m_objects; }

		protected:
			std::unordered_set<TypePtr> m_objects;
			mutable std::mutex m_lock;
		};

		/// @brief Empty base class used for tracking elsewhere.
		class IRHI_ResourceCache
		{
		public:
			virtual ~IRHI_ResourceCache() { }

			virtual void Release() = 0;
		};

		/// @brief Setup a small cache object which allows for fast access to a given resource type.
		/// This is like a map.
		/// @tparam TValue 
		template<typename TValue>
		class RHI_ResourceCache : public IRHI_ResourceCache
		{
			THREAD_SAFE;
			static_assert(std::is_base_of_v<RHI_Resource, TValue>);
			static_assert(!std::is_pointer_v<TValue>);

			using TypePtr = TValue*;

			struct Item
			{
				TypePtr	ItemPtr;
				int Id;
			};

		public:
			RHI_ResourceCache(RHI_ResourceManager<TValue>& manager)
				: m_manager(manager)
			{ }

			int AddOrReturn(std::string str)
			{
				std::lock_guard lock(m_lock);
				auto itr = m_itemLookup.find(str);
				if (itr != m_itemLookup.end())
				{
					return itr->second.Id;
				}

				TypePtr ptr = m_manager.CreateResource();
				ptr->SetName(str);
				int id = GetFreeId();

				m_itemLookup[str] = Item { ptr, id };
				m_idToStrLookup[id] = str;

				return id;
			}

			void ReleaseResource(int id)
			{
				std::lock_guard lock(m_lock);

				auto idToStrItr = m_idToStrLookup.find(id);
				if (idToStrItr == m_idToStrLookup.end())
				{
					return;
				}

				auto strToResourceItr = m_itemLookup.find(idToStrItr->second);
				if (strToResourceItr == m_itemLookup.end())
				{
					return;
				}
				TypePtr resource = strToResourceItr->second.ItemPtr;
				m_manager.FreeResource(resource);
				ReturnId(id);

				m_idToStrLookup.erase(idToStrItr);
				m_itemLookup.erase(strToResourceItr);
			}

			TypePtr Get(std::string key)
			{
				std::lock_guard lock(m_lock);
				if (auto itr = m_itemLookup.find(key); itr != m_itemLookup.end())
				{
					return itr->second.ItemPtr;
				}
				return nullptr;
			}

			TypePtr Get(int key)
			{
				std::lock_guard lock(m_lock);
				if (auto itr = m_idToStrLookup.find(key); itr != m_idToStrLookup.end())
				{
					return m_itemLookup.at(itr->second).ItemPtr;
				}
				return nullptr;
			}

			int GetId(std::string key)
			{
				std::lock_guard lock(m_lock);
				if (auto itr = m_itemLookup.find(key); itr != m_itemLookup.end())
				{
					return m_itemLookup.at(key).Id;
				}
				return -1;
			}
			
			bool HasValue(TypePtr ptr) const
			{
				std::lock_guard lock(m_lock);
				for (auto& [key, value] : m_itemLookup)
				{
					if (value.ItemPtr == ptr)
					{
						return true;
					}
				}
				return false;
			}

			virtual void Release() override
			{
				std::lock_guard lock(m_lock);
				for (const auto& [str, item] : m_itemLookup)
				{
					m_manager.FreeResource(item.ItemPtr);
				}
				m_itemLookup.clear();
				m_idToStrLookup.clear();
				m_currentMaxId = 0;
			}

		private:
			int GetFreeId()
			{
				// Must be locked externally.
				if (m_freeIds.empty())
				{
					m_freeIds.push(m_currentMaxId++);
				}
				int freeId = m_freeIds.front();
				m_freeIds.pop();
				return freeId;
			}

			void ReturnId(int id)
			{
				// Must be locked externally.
				m_freeIds.push(id);
			}

		private:
			mutable std::mutex m_lock;

			RHI_ResourceManager<TValue>& m_manager;

			int m_currentMaxId = 0;
			std::queue<int> m_freeIds;
			std::unordered_map<std::string, Item> m_itemLookup;
			std::unordered_map<int, std::string> m_idToStrLookup;
		};
	}
}