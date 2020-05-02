#include "ispch.h"
#include "Entity.h"

#include "Insight/Module/EntityModule.h"
#include "Insight/Component/Component.h"
#include "Insight/UUID.h"

#include "Insight/Log.h"

using namespace Insight::Module;

Entity::Entity()
{
	m_data.UUID = Insight::GenUUID();
	m_data.Name = "Default";
	EntityModule::m_entities.push_back(this);
}

Entity::Entity(const std::string& id)
{
	m_data.UUID = Insight::GenUUID();
	m_data.Name = id;
	EntityModule::m_entities.push_back(this);
}

Entity::~Entity()
{
	for (auto it = m_data.Components.begin(); it != m_data.Components.end(); ++it)
	{
		Insight::Memory::MemoryManager::DeleteOnFreeList(*it);
	}
	m_data.Components.clear();

	EntityModule::m_entities.erase(std::find(EntityModule::m_entities.begin(), EntityModule::m_entities.end(), this));
}

void Entity::SetID(const std::string& id)
{
	m_data.Name = id;
}

const std::string& Entity::GetID() const
{
	return m_data.Name;
}
