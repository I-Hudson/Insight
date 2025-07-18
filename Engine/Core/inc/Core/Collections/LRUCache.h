#pragma once
#if 1
#include "Core/TypeAlias.h"
#include "Core/NonCopyable.h"

#include <list>
#include <unordered_map>

namespace Insight
{
    namespace Core
    {
        template<typename TKey, typename TValue, u32 Size>
        class LRUCache : NonCopyable
        {
            struct Item
            {
                TKey Key;
                TValue Value;
            };

        public:
            LRUCache()
            {
                m_lookup.reserve(Size);
            }
            LRUCache(LRUCache&& other)
            {
                m_items = std::move(other.m_items);
                m_lookup = std::move(other.m_lookup);

                other = {};
            }
            ~LRUCache()
            {

            }

            LRUCache& operator=(LRUCache&& other)
            {
                m_items = std::move(other.m_items);
                m_lookup = std::move(other.m_lookup);

                other = {};
            }

            void Put(const TKey& key, const TValue& value)
            {
                if (auto iter = m_lookup.find(key);
                    iter != m_lookup.end())
                {
                    m_items.erase(iter->second);
                    m_items.push_front({ key, value });
                    iter->second = m_items.begin();
                }
                else
                {
                    if (m_items.size() == Size)
                    {
                        m_lookup.erase(m_items.back().Key);
                        m_items.pop_back();
                    }

                    m_items.push_front({ key, value });
                    m_lookup[key] = m_items.begin();
                }
            }

            bool Get(const TKey& key, TValue& value) const
            {
                if (auto iter = m_lookup.find(key);
                    iter != m_lookup.end())
                {
                    value = iter->second->Value;
                    return true;
                }
                
                return false;
            }

        private:
            std::list<Item> m_items;
            // https://stackoverflow.com/a/11275548
            std::unordered_map<TKey, typename std::list<Item>::iterator> m_lookup;
        };
    }
}
#endif