#pragma once

#include "Core/Defines.h"

#include <typeinfo>

namespace Insight
{
	namespace Core
	{
		struct IS_CORE IProxyHandle
		{
		public:

		private:
			int m_index = 0;
			int m_generation = 0;

			template<typename ProxyHandle, typename Type>
			friend class ProxyAllocator;
		};

		/// @brief The Proxy Allcator is an allocator which binds a proxy handle (POD struct) to 
		/// a type stored (normally in a vector) as to remove the need to pass pointers around a
		/// handle is used instead. This allows the handle to act as a weak pointer and removes the
		/// danger of raw pointes being handled by system which shouldn't have them.
		/// @tparam ProxyHandle 
		/// @tparam Type 
		template<typename ProxyHandle, typename Type>
		class IS_CORE ProxyAllocator
		{
			static_assert(std::is_base_of_v<IProxyHandle, ProxyHandle>, "[ProxyAllocator] 'ProxyHandle' must inherit from 'IProxyHandle'.");
		public:

		private:
		};
	}
}