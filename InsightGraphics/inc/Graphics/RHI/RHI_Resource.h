#pragma once

#include "Core/Logger.h"
#include "Core/Memory.h"
#include <type_traits>
#include <map>

namespace Insight
{
	namespace Graphics
	{
		class RHI_Resource
		{
		public:
			virtual void Release() = 0;
		};

		template<typename T>
		class RHI_ResourceManager
		{
			using Type = T;
			using TypePtr = Type*;

		public:
			RHI_ResourceManager()
			{
				static_assert(!std::is_pointer_v<T>, "[RHI_ResourceManager] 'T' must not be a pointer.");
			}

			~RHI_ResourceManager()
			{
				IS_CORE_INFO("Allocs: {}", m_objects.size());
			}

			TypePtr CreateResource()
			{
				TypePtr object = Type::New();
				m_objects[object] = object;
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
				for (const auto& pair : m_objects)
				{
					if (pair.second)
					{
						pair.second->Release();
						DeleteTracked(pair.second);
					}
				}
				m_objects.clear();
			}

		private:
			std::map<TypePtr, TypePtr> m_objects;
		};
	}
}