#include "ispch.h"
#include "MeshComponent.h"

#include "Insight/Module/GraphicsModule.h"

MeshComponent::MeshComponent()
	: Component(nullptr, ComponentType::MESH)
	, m_mesh(nullptr)
{
}

MeshComponent::MeshComponent(Entity* owner)
	: Component(owner, ComponentType::MESH)
	, m_mesh(nullptr)
{
	Insight::Module::GraphicsModule::m_meshs.push_back(this);
}

MeshComponent::~MeshComponent()
{
	Insight::Module::GraphicsModule::m_meshs.erase(std::find(Insight::Module::GraphicsModule::m_meshs.begin(), Insight::Module::GraphicsModule::m_meshs.end(), this));
}

void MeshComponent::SetMaterial(Material* material)
{
	if (m_materal != nullptr)
	{
		--m_materal->m_useageCount;
		m_materal->m_isDirty = true;
	}

	m_materal = material;

	if (m_materal != nullptr)
	{
		++m_materal->m_useageCount;
		m_materal->m_isDirty = true;

	}
}

void MeshComponent::Serialize(std::ostream& out)
{
	out << "MeshComponent: " << GetUUID() ENDL;
	out << "MeshName:" << m_mesh->GetName() ENDL;
}

void MeshComponent::Deserialize(std::istream& in)
{

}