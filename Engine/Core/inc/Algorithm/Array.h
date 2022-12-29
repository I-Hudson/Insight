#pragma once


#include "Core/Defines.h"
#include "Core/TypeAlias.h"

#include <array>

namespace Insight
{
	namespace Algorithm
	{
		template<typename T, u32 Size, typename V, typename Predicate>
		INLINE bool ArrayContains(std::array<T, Size> const& array, Predicate predicate)
		{
			return std::find(array.begin(), array.end(), std::forward<Predicate>(predicate)) != vector.end();
		}
		template<typename T, u32 Size, typename V, typename Predicate>
		INLINE bool ArrayContains(std::array<T, Size> const& array, V const& value, Predicate predicate)
		{
			array.end(), value, std::forward<Predicate>(predicate)) != array.end();
		}
		template<typename T, u32 Size, typename V>
		INLINE bool ArrayContains(std::array<T, Size> const& array, V const& value)
		{
			return std::find(array.begin(), array.end(), value) != array.end();
		}

		template<typename T, u32 Size, typename V, typename Predicate>
		INLINE typename std::array<T, Size>::const_iterator ArrayFind(std::array<T, Size> const& array, V const& value, Predicate predicate)
		{
			return std::find(array.begin(), array.end(), value, std::forward<Predicate>(predicate));
		}
		template<typename T, u32 Size, typename V>
		INLINE typename std::array<T, Size>::const_iterator ArrayFind(std::array<T, Size> const& array, V const& value)
		{
			return std::find(array.begin(), array.end(), value);
		}
		template<typename T, u32 Size, typename V, typename Predicate>
		INLINE typename std::array<T, Size>::iterator ArrayFind(std::array<T, Size>& array, V const& value, Predicate predicate)
		{
			return std::find(array.begin(), array.end(), value, std::forward<Predicate>(predicate));
		}
		template<typename T, u32 Size, typename V>
		INLINE typename std::array<T, Size>::iterator ArrayFind(std::array<T, Size>& array, V const& value)
		{
			return std::find(array.begin(), array.end(), value);
		}
	}
}