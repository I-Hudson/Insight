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

		//bool Entity::HasComponent(u64 componentType) const
		//{
		//	if (!IsVaild())
		//	{
		//		return false;
		//	}
		//	ASSERT(m_entityManager)
		//	const EntityData& data = m_entityManager->GetEntityData(*this);
		//	return data.Components.find(componentType) != data.Components.end()l;
		//}

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

		Core::GUID Entity::GetGuid() const
		{
			if (!IsVaild())
			{
				return Core::GUID::s_InvalidGUID;
			}
			ASSERT(m_entityManager); 
			return m_entityManager->GetEntityData(*this).GUID;
		}
	}
}