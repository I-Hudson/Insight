#pragma once

#include "Core/Defines.h"

#include <vector>

namespace Insight
{
	namespace Algorithm
	{
		template<typename T, typename V, typename Predicate>
		INLINE bool VectorContains(std::vector<T> const& vector, Predicate predicate)
		{
			return std::find(vector.begin(), vector.end(), std::forward<Predicate>(predicate)) != vector.end();
		}
		template<typename T, typename V, typename Predicate>
		INLINE bool VectorContains(std::vector<T> const& vector, V const& value, Predicate predicate)
		{
			return std::find(vector.begin(), vector.end(), value, std::forward<Predicate>(predicate)) != vector.end();
		}
		template<typename T, typename V>
		INLINE bool VectorContains(std::vector<T> const& vector, V const& value)
		{
			return std::find(vector.begin(), vector.end(), value) != vector.end();
		}

		template<typename T, typename V, typename Predicate>
		INLINE typename std::vector<T>::const_iterator VectorFind(std::vector<T> const& vector, V const& value, Predicate predicate)
		{
			return std::find(vector.begin(), vector.end(), value, std::forward<Predicate>(predicate));
		}
		template<typename T, typename V>
		INLINE typename std::vector<T>::const_iterator VectorFind(std::vector<T> const& vector, V const& value)
		{
			return std::find(vector.begin(), vector.end(), value);
		}
		template<typename T, typename V, typename Predicate>
		INLINE typename std::vector<T>::iterator VectorFind(std::vector<T>& vector, V const& value, Predicate predicate)
		{
			return std::find(vector.begin(), vector.end(), value, std::forward<Predicate>(predicate));
		}
		template<typename T, typename V>
		INLINE typename std::vector<T>::iterator VectorFind(std::vector<T>& vector, V const& value)
		{
			return std::find(vector.begin(), vector.end(), value);
		}
	}
}