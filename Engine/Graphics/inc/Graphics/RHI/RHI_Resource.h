#pragma once

#include "Core/Memory.h"
#include "Core/TypeAlias.h"
#include "Graphics/Enums.h"
#include "Graphics/Defines.h"

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
			DeviceUploadStatus m_uploadStatus = DeviceUploadStatus::Unknown;
			mutable std::mutex m_mutex;

			friend struct RHI_UploadQueueRequestInternal;
		};

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
				std::lock_guard lock(m_lock);
				auto itr = m_objects.find(object);
				if (itr != m_objects.end())
				{
					m_objects.erase(itr);
					if (object)
					{
						object->Release();
						Delete(object);
					}
				}
			}

			void ReleaseAll()
			{
				std::lock_guard lock(m_lock);
				for (auto obj : m_objects)
				{
					if (obj)
					{
						obj->Release();
						DeleteTracked(obj);
					}
				}
				m_objects.clear();
			}

			int GetSize() const { return static_cast<int>(m_objects.size()); }

		protected:
			std::unordered_set<TypePtr> m_objects;
			std::mutex m_lock;
		};

		template<typename TValue>
		class RHI_ResourceCache : public RHI_ResourceManager<TValue>
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

			int AddOrReturn(std::string str)
			{
				std::lock_guard lock(m_lock);
				auto itr = m_itemLookup.find(str);
				if (itr != m_itemLookup.end())
				{
					return itr->second.Id;
				}

				TypePtr ptr = RHI_ResourceManager<TValue>::CreateResource();
				ptr->SetName(str);
				int id = GetFreeId();

				m_itemLookup[str] = Item { ptr, id };
				m_idToStrLookup[id] = str;

				return id;
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

			void Reset()
			{
				std::lock_guard lock(m_lock);
				m_itemLookup.clear();
				m_idToStrLookup.clear();
				m_currentMaxId = 0;
				RHI_ResourceManager<TValue>::ReleaseAll();
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
			std::mutex m_lock;
			std::unordered_map<std::string, Item> m_itemLookup;

			int m_currentMaxId = 0;
			std::queue<int> m_freeIds;
			std::unordered_map<int, std::string> m_idToStrLookup;
		};
	}
}