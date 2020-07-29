#include "ispch.h"
#include "Entity.h"

#include "Insight/Module/EntityModule.h"
#include "Insight/Component/Component.h"
#include "Insight/Component/MeshComponent.h"
#include "Insight/Assimp/Model.h"
#include "Insight/Instrumentor/Instrumentor.h"

#include "Insight/Log.h"

using namespace Insight::Module;

Entity::Entity()
	: Insight::UUID()
{
	m_data.Name = "Default";
}

Entity::Entity(const std::string& id)
	: Insight::UUID()
{
	m_data.Name = id;
}

Entity::~Entity()
{
	RemoveAllComponenets();
}

Entity* Entity::Create(const std::string& id)
{
	return EntityModule::GetInstance()->Create(id);
}

Entity* Entity::CreateFromModel(Model* model)
{
	IS_PROFILE_FUNCTION();

	std::string modelName = model->GetName();
	Entity* e = Entity::Create(modelName);

	unsigned int meshCount = model->GetSubMeshCount();
	for (unsigned int i = 0; i < meshCount; ++i)
	{
		Mesh* subMesh = model->GetSubMesh(i);
		Entity* childEntity = Entity::Create(subMesh->GetName());
		childEntity->AddComponent<MeshComponent>()->SetMesh(subMesh);

		e->AddChild(childEntity);
	}

	return e;
}

void Entity::Delete()
{
	IS_PROFILE_FUNCTION();

	if (this != nullptr)
	{
		for (int i = 0; i < m_data.Children.size(); ++i)
		{
			EntityModule::GetInstance()->Delete(m_data.Children[i]);
		}
		EntityModule::GetInstance()->Delete(this);
	}
}

void Entity::SetID(const std::string& id)
{
	m_data.Name = id;
}

const std::string& Entity::GetID() const
{
	return m_data.Name;
}

Entity* Entity::AddChild(const std::string& childId)
{
	Entity* e = EntityModule::GetInstance()->Create(childId);
	AddChild(e);
	return e;
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
	if (childIndex < 0 || childIndex >= m_data.Children.size())
	{
		IS_ASSERT(true, "Entity: GetChild: Out of range.");
		return nullptr;
	}
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

void Entity::RemoveAllComponenets()
{
	IS_PROFILE_FUNCTION();

	for (auto it = m_data.Components.begin(); it != m_data.Components.end(); ++it)
	{
		DELETE_ON_HEAP((*it));
	}
	m_data.Components.clear();
	m_data.ComponetBitset.reset();
}
