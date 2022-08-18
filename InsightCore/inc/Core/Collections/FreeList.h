#pragma once

#include <vector>
#include <queue>
#include <unordered_map>

namespace Insight
{
	namespace Core
	{
		template<typename T>
		class FreeList;

		class FreeListHandle
		{
		public:

		private:
		};

		template<typename T>
		class FreeList
		{
		public:
			void RemoveItem(T& item);

		private:
			std::vector<T> m_items;
			std::queue<int> m_freePlaces;
			std::unordered_map< T, int> m_itemToIndex;
		};
	}
}