#include "ispch.h"
#include "EntityModule.h"

#include "Insight/Memory/MemoryManager.h"
#include "Insight/Entitiy/Entity.h"

namespace Insight
{
	namespace Module
	{
		std::vector<Entity*> EntityModule::m_entities;

		EntityModule::~EntityModule()
		{
			while(m_entities.size() > 0)
			{
				Memory::MemoryManager::DeleteOnFreeList(*m_entities.begin());
			}
			m_entities.clear();
		}

		void EntityModule::Update(const float& deltaTime)
		{ }
	}
}
