#include "ispch.h"
#include "EntityModule.h"

#include "Insight/Memory/MemoryManager.h"
#include "Insight/Entitiy/Entity.h"
#include "Insight/Component/TransformComponent.h"

namespace Insight
{
	namespace Module
	{
		std::vector<Entity*> EntityModule::m_entities;

		EntityModule::EntityModule()
		{
			TSingleton::SetInstancePtr(this);
		}

		EntityModule::~EntityModule()
		{
			while(m_entities.size() > 0)
			{
				Delete(*m_entities.begin());
			}
			m_entities.clear();
			TSingleton::ClearPtr();
		}

		void EntityModule::Update(const float& deltaTime)
		{ }

		Entity* EntityModule::Create(const std::string & id)
		{
			Entity* e = NEW_ON_HEAP(Entity, id);
			e->AddComponent<TransformComponent>();

			return e;
		}

		void EntityModule::Delete(Entity* ptr)
		{
			auto it = std::find(m_entities.begin(), m_entities.end(), ptr);
			DELETE_ON_HEAP((*it));

			m_entities.erase(it);
		}
	}
}
