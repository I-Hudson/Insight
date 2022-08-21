#pragma once

#include "Core/Memory.h"
#include "Core/TypeAlias.h"
#include "Graphics/Enums.h"
#include <type_traits>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <string>

namespace Insight
{
	namespace Graphics
	{
		class RHI_Resource
		{
		public:
			virtual ~RHI_Resource() { }

			virtual void Release() = 0;
			virtual bool ValidResouce() = 0;
			virtual void SetName(std::wstring name) = 0;

		public:
			std::wstring m_name;
		};

		template<typename T>
		class RHI_ResourceManager
		{
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
				TypePtr object = Type::New();
				m_objects.insert(object);
				return object;
			}

			void FreeResource(TypePtr object)
			{
				auto itr = m_objects.find(object);
				if (itr != m_objects.end())
				{
					m_objects.erase(itr);
					if (object)
					{
						object->Release();
						DeleteTracked(object);
					}
				}
			}

			void ReleaseAll()
			{
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
		};

		template<typename TValue>
		class RHI_ResourceCache : public RHI_ResourceManager<TValue>
		{
			static_assert(std::is_base_of_v<RHI_Resource, TValue>);

			struct Item
			{
				TypePtr	ItemPtr;
				int Id;
			};

		public:

			int AddOrReturn(std::wstring str)
			{
				auto itr = m_itemLookup.find(str);
				if (itr != m_itemLookup.end())
				{
					return itr->second.Id;
				}

				TypePtr ptr = CreateResource();
				ptr->SetName(str);
				int id = GetFreeId();

				m_itemLookup[str] = Item { ptr, id };
				m_idToStrLookup[id] = str;

				return id;
			}

			TypePtr Get(std::wstring key) const
			{
				if (auto itr = m_itemLookup.find(key); itr != m_itemLookup.end())
				{
					return itr->second.ItemPtr;
				}
				return nullptr;
			}

			TypePtr Get(int key) const
			{
				if (auto itr = m_idToStrLookup.find(key); itr != m_idToStrLookup.end())
				{
					return m_itemLookup.at(itr->second).ItemPtr;
				}
				return nullptr;
			}

			int GetId(std::wstring key) const
			{
				if (auto itr = m_itemLookup.find(key); itr != m_itemLookup.end())
				{
					return m_itemLookup.at(key).Id;
				}
				return -1;
			}

			void Reset()
			{
				m_itemLookup.clear();
				m_idToStrLookup.clear();
				m_currentMaxId = 0;
				ReleaseAll();
			}

		private:
			int GetFreeId()
			{
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
				m_freeIds.push(id);
			}

		private:
			std::unordered_map<std::wstring, Item> m_itemLookup;

			int m_currentMaxId = 0;
			std::queue<int> m_freeIds;
			std::unordered_map<int, std::wstring> m_idToStrLookup;
		};
	}
}