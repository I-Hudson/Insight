#include "ispch.h"
#include "Entity.h"

#include "Insight/Module/EntityModule.h"
#include "Insight/Component/Component.h"

#include "Insight/Log.h"

using namespace Insight::Module;

Entity::Entity()
	: Insight::UUID()
{
	m_data.Name = "Default";
	EntityModule::m_entities.push_back(this);
}

Entity::Entity(const std::string& id)
	: Insight::UUID()
{
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

void Entity::AddChild(Entity* child)
{
	if (std::find(m_data.Children.begin(), m_data.Children.end(), child) == m_data.Children.end())
	{
		m_data.Children.push_back(child);
	}
}

Entity* Entity::GetChild(int childIndex)
{
	assert("Entity: GetChild: Out of range!", childIndex >= m_data.Children.size());
	return m_data.Children[childIndex];
}

void Entity::RemoveChild(Entity* child)
{
	auto it = std::find(m_data.Children.begin(), m_data.Children.end(), child);
	if (it != m_data.Children.end())
	{
		m_data.Children.erase(it);
		child->SetParent(nullptr);
	}
}
