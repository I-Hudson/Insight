#pragma once

#include "Core/TypeAlias.h"

namespace Insight
{
	namespace ECS
	{
		template<typename>
		class ComponentArray;

		class ComponentHandle
		{
		public:
			ComponentHandle() { }
			ComponentHandle(int index, u64 componentType) : m_index(index), m_componentType(componentType) { }

			int GetIndex() const { return m_index; }
			u64 GetType() const { return m_componentType; }

			bool IsValid() const { return m_index != -1; }

		private:
			int m_index = -1;
			u64 m_componentType = 0;

			template<typename>
			friend class ComponentArray;
		};
	}
}