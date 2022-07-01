#include "ECS/Entity.h"
#include "ECS/EntityManager.h"

namespace Insight
{
	namespace ECS
	{
		void Entity::SetName(std::string name)
		{
			if (!IsVaild())
			{
				return;
			}
			ASSERT(m_entityManager);
			m_entityManager->GetEntityData(*this).Name = name;
		}

		bool Entity::IsVaild() const
		{
			return m_id != -1;
		}

		std::string Entity::GetName() const
		{
			if (!IsVaild())
			{
				return "";
			}
			ASSERT(m_entityManager);
			return m_entityManager->GetEntityData(*this).Name;
		}

		int Entity::GetId() const
		{
			return m_id;
		}

		u64 Entity::GetGuid() const
		{
			if (!IsVaild())
			{
				return static_cast<u64>(-1);
			}
			ASSERT(m_entityManager); 
			return m_entityManager->GetEntityData(*this).GUID;
		}
	}
}