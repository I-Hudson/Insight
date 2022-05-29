#pragma once

#include <initializer_list>
#include <type_traits>
#include <vector>

namespace Insight
{
	namespace Core
	{
		template<typename T>
		struct Slice
		{
			using TType = T;
			using TRef = TType&;
			using TPtr = const TType*;

		public:
			Slice(TPtr begin, TPtr end)
				: m_begin(begin), m_size(static_cast<int>((end - begin)))
			{ }

			Slice(std::initializer_list<T> list)
				: m_begin(list.begin()), m_size(static_cast<int>(list.size()))
			{ }

			Slice(std::vector<T> const& vec)
				: m_begin(vec.begin()), m_size(static_cast<int>(vec.size()))
			{ }

			int GetSize() const { return m_size; }
			TPtr GetBegin() const { return m_begin; }
			TPtr GetEnd() const { return m_begin + (sizeof(T) * m_size); }
				
		private:
			TPtr m_begin = nullptr;
			int m_size = 0;
		};
	}
}