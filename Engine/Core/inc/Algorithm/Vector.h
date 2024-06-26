#pragma once

#include "Core/Defines.h"
#include "Core/TypeAlias.h"

#include <vector>

namespace Insight
{
	namespace Algorithm
	{
		template<typename T, typename Predicate>
		INLINE bool VectorContainsIf(std::vector<T> const& vector, Predicate predicate)
		{
			return std::find_if(vector.begin(), vector.end(), std::forward<Predicate>(predicate)) != vector.end();
		}

		template<typename T, typename V>
		INLINE bool VectorContains(std::vector<T> const& vector, V const& value)
		{
			return std::find(vector.begin(), vector.end(), value) != vector.end();
		}

		template<typename T, typename V>
		INLINE typename std::vector<T>::const_iterator VectorFind(std::vector<T> const& vector, V const& value)
		{
			return std::find(vector.begin(), vector.end(), value);
		}
		template<typename T, typename V>
		INLINE typename std::vector<T>::iterator VectorFind(std::vector<T>& vector, V const& value)
		{
			return std::find(vector.begin(), vector.end(), value);
		}

		template<typename T, typename Predicate>
		INLINE typename std::vector<T>::const_iterator VectorFindIf(std::vector<T> const& vector, Predicate predicate)
		{
			return std::find_if(vector.begin(), vector.end(), std::forward<Predicate>(predicate));
		}
		template<typename T, typename Predicate>
		INLINE typename std::vector<T>::iterator VectorFindIf(std::vector<T>& vector,  Predicate predicate)
		{
			return std::find_if(vector.begin(), vector.end(), std::forward<Predicate>(predicate));
		}

		template<typename T, typename Predicate>
		INLINE typename std::vector<T>::const_iterator VectorFindAllIf(std::vector<T> const& vector, Predicate predicate)
		{
			std::vector<typename std::vector<T>::const_iterator> result;
			for (size_t i = 0; i < vector.size(); ++i)
			{
				T const& iter = vector.at(i);
				if (predicate(iter))
				{
					result.push_back(vector.begin() + i);
				}
			}
			return result;
		}		
		template<typename T, typename Predicate>
		INLINE typename std::vector<typename std::vector<T>::iterator> VectorFindAllIf(std::vector<T>& vector, Predicate predicate)
		{
			std::vector<typename std::vector<T>::iterator> result;
			for (size_t i = 0; i < vector.size(); ++i)
			{
				T const& iter = vector.at(i);
				if (predicate(iter))
				{
					result.push_back(vector.begin() + i);
				}
			}
			return result;
		}

		template<typename T, typename V>
		INLINE typename std::vector<typename std::vector<T>::const_iterator> VectorFindAll(std::vector<T> const& vector, V const& value)
		{
			std::vector<typename std::vector<T>::const_iterator> result;
			for (size_t i = 0; i < vector.size(); ++i)
			{
				T const& iter = vector.at(i);
				if (iter == value)
				{
					result.push_back(vector.begin() + i);
				}
			}
			return result;
		}
		template<typename T, typename V>
		INLINE typename std::vector<typename std::vector<T>::iterator> VectorFindAll(std::vector<T>& vector, V const& value)
		{
			std::vector<typename std::vector<T>::iterator> result;
			for (size_t i = 0; i < vector.size(); ++i)
			{
				T const& iter = vector.at(i);
				if (iter == value)
				{
					result.push_back(vector.begin() + i);
				}
			}
			return result;
		}

		template<typename T, typename V>
		INLINE void VectorAddUnique(std::vector<T>& vector, V const& value)
		{
			if (VectorFind(vector, value) == vector.end())
			{
				vector.push_back(value);
			}
		}

		template<typename T, typename Predicate>
		INLINE typename std::vector<T>::iterator VectorRemoveIf(std::vector<T>& vector, Predicate predicate)
		{
			for (size_t i = 0; i < vector.size(); ++i)
			{
				if (predicate(vector.at(i)))
				{
					return vector.erase(vector.begin() + i);
				}
			}
			return vector.end();
		}
		template<typename T, typename Predicate>
		INLINE void VectorRemoveAllIf(std::vector<T>& vector, Predicate predicate)
		{
			auto iter = std::remove_if(vector.begin(), vector.end(), predicate);
			vector.erase(iter, vector.end());
		}

		template<typename T>
		INLINE typename std::vector<T>::iterator VectorRemove(std::vector<T>& vector, typename std::vector<T>::const_iterator const& iterator)
		{
			if (iterator >= vector.begin() && iterator < vector.end())
			{
				return vector.erase(iterator);
			}
			return vector.end();
		}
		template<typename T, typename V>
		INLINE bool VectorRemove(std::vector<T>& vector, V const& value)
		{
			auto iter = VectorFind(vector, value);
			if (iter != vector.end())
			{
				vector.erase(iter);
				return true;
			}
			return false;
		}

		template<typename T, typename V, std::enable_if_t<std::is_integral_v<V>, bool> = true>
		INLINE bool VectorIndexWithinRange(const std::vector<T>& vector, const V index)
		{
			if (index < 0 || (!vector.empty() && index > vector.size() - 1))
			{
				return false;
			}
			return true;
		}
	}
}