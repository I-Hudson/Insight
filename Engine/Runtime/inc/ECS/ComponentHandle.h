#pragma once

#include "Core/TypeAlias.h"
#include <type_traits>

namespace Insight
{
	namespace ECS
	{
#ifdef IS_ECS_ENABLED
		template<typename Component>
		u64 GetComponentId()
		{
			return typeid(Component).hash_code();
		}

		template<typename>
		class ComponentArray;

		class ComponentHandle
		{
		public:
			ComponentHandle() { }
			ComponentHandle(int ownerEntity, int index, u64 componentType) 
				: m_ownerEntity(ownerEntity), m_index(index), m_componentType(componentType) 
			{ }

			int GetIndex() const { return m_index; }
			u64 GetType() const { return m_componentType; }

			bool IsValid() const { return m_index != -1; }

		private:
			int m_ownerEntity = -1;
			int m_index = -1;
			u64 m_componentType = 0;

			template<typename>
			friend class ComponentArray;
		};
#endif
	}
}