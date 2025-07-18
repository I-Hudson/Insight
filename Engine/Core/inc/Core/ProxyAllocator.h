#pragma once

#include "Core/Defines.h"
#include "Core/NonCopyable.h"

#include <typeinfo>

namespace Insight
{
	namespace
	{
		template<typename T>
		class ProxyAllocator;

		template<typename T>
		struct ProxyHandle
		{
		private:
			using PAllocator = ProxyAllocator<T>;
		public:
			ProxyHandle() = delete;
			ProxyHandle(const int index, const int generation, PAllocator* proxyAllocator)
				: m_index(index)
				, m_generation(generation)
				, m_allocator(proxyAllocator)
			{ }

			bool IsValid() const
			{
				if (!m_allocator)
				{
					return false;
				}

				ASSERT(m_allocator);
				return m_allocator->IsValidHandle(*this);
			}


		private:
			int m_index = -1;
			int m_generation = -1;
			PAllocator* m_allocator = nullptr;

			friend PAllocator;
		};

		/// @brief The Proxy Allocator is an allocator which binds a proxy handle (POD struct) to 
		/// a type stored (normally in a vector) as to remove the need to pass pointers around a
		/// handle is used instead. This allows the handle to act as a weak pointer and removes the
		/// danger of raw pointes being handled by system which shouldn't have them.
		/// @tparam ProxyHandle 
		/// @tparam Type 
		template<typename Type>
		class ProxyAllocator : NonCopyable
		{
			//using TypeIsPointer = std::is_pointer_v<Type>;
			using Handle = ProxyHandle<Type>;

			//static_assert(std::is_pod_v<Handle>, "[ProxyAllocator] 'ProxyHandle' must be POD'.");
			static_assert(!std::is_reference_v<Type>, "[ProxyAllocator] 'Type' can't be a reference type.");

		public:
			ProxyAllocator()
			{ }
			~ProxyAllocator()
			{
				ASSERT(m_freeList.size() == m_objects.size());
			}

			template<std::enable_if_t<std::is_default_constructible_v<Type>, int> = 0>
			Handle Allocate()
			{
				if constexpr (std::is_pointer_v<Type>)
				{
					return Allocate(::New<Type>());
				}
				else
				{
					return Allocate(Type());
				}
			}
			Handle Allocate(Type type)
			{
				if (!m_freeList.empty())
				{
					Handle freeHandle = m_freeList.back();
					m_freeList.pop_back();

					m_objects[freeHandle.m_index] = type;
					return freeHandle;
				}

				const int index = (int)m_objects.size();
				m_objects.emplace_back(type);
				m_generations.emplace_back(1);

				Handle handle(index, 1, this);
				return handle;
			}

			void Release(Handle& handle)
			{
				if (!IsValidHandle(handle))
				{
					return;
				}

				int& gen = m_generations[handle.m_index];
				++gen;

				if constexpr (std::is_pointer_v<Type>)
				{
					::Delete(m_objects[handle.m_index]);
				}
				m_freeList.emplace_back(Handle(handle.m_index, gen, this));
				handle = Handle(-1, -1, nullptr);
			}

			bool IsValidHandle(const Handle handle) const
			{
				if (handle.m_index < 0 || handle.m_index >= m_objects.size())
				{
					return false;
				}

				const int gen = m_generations[handle.m_index];
				ASSERT(handle.m_generation <= gen);
				return handle.m_generation == gen;
			}

			Type GetType(const Handle handle)
			{
				return const_cast<const ProxyAllocator*>(this)->GetType(handle);
			}
			const Type GetType(const Handle handle) const
			{
				if (!IsValidHandle(handle))
				{
					if constexpr(std::is_pointer_v<Type>)
					{
						return nullptr;
					}
					else
					{
						return Type();
					}
				}

				return m_objects[handle.m_index];
			}

		private:
			std::vector<int> m_generations;
			std::vector<Type> m_objects;
			std::list<Handle> m_freeList;
		};
	}
}