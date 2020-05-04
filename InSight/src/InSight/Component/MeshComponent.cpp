#include "ispch.h"
#include "MeshComponent.h"

#include "Insight/Module/GraphicsModule.h"

MeshComponent::MeshComponent(Entity* owner)
	: Component(owner, ComponentType::MESH), m_mesh(nullptr)
{
	Insight::Module::GraphicsModule::m_meshs.push_back(this);
}

MeshComponent::~MeshComponent()
{
	Insight::Module::GraphicsModule::m_meshs.erase(std::find(Insight::Module::GraphicsModule::m_meshs.begin(), Insight::Module::GraphicsModule::m_meshs.end(), this));
}
