#include "ispch.h"
#include "MeshComponent.h"

#include "Insight/Module/GraphicsModule.h"
#include "Insight/Library/ModelLibrary.h"
#include "Insight/Model/Model.h"

REGISTER_DEF_TYPE(MeshComponent);

MeshComponent::MeshComponent()
	: Component(nullptr)
{
	m_updateEveryFarme = false;
}

MeshComponent::MeshComponent(SharedPtr<Entity> owner)
	: Component(owner)
{
	m_updateEveryFarme = false;
}

MeshComponent::~MeshComponent()
{
}

void MeshComponent::OnCreate()
{
	__super::OnCreate();

	IS_PROPERTY(std::string, m_meshName, "Mesh Name", ShowInEditor | ReadOnly);

	Insight::Module::GraphicsModule::m_meshs.push_back(this);
	//SetMaterial(Insight::Module::GraphicsModule::GetDefaultMaterial());
	m_updateEveryFarme = false;
}

void MeshComponent::OnDestroy()
{
	Insight::Module::GraphicsModule::m_meshs.erase(std::find(Insight::Module::GraphicsModule::m_meshs.begin(), Insight::Module::GraphicsModule::m_meshs.end(), this));
}

void MeshComponent::SetMesh(WeakPtr<Mesh> mesh)
{
	m_mesh = mesh;
	m_meshName = mesh.lock()->GetMeshName();
}

void MeshComponent::SetMaterial(WeakPtr<Material> material, int index)
{

}

void MeshComponent::SetMaterials(std::vector<WeakPtr<Material>> material, int index)
{
}

void MeshComponent::Serialize(SharedPtr<Insight::Serialization::SerializableElement> element, bool force)
{
	element->AddAttribute("UUID", GetUUID());
	if (auto meshSP = m_mesh.lock())
	{
		//element->AddAttribute("MeshName", meshSP->GetName().c_str());
	}
}

void MeshComponent::Deserialize(SharedPtr<Insight::Serialization::SerializableElement> element, bool force)
{
	//using namespace Insight::Library;
	//SetMesh(ModelLibrary::Instance()->GetAsset(in->FirstChildElement("ModelUUID")->GetText())->GetSubMesh(in->FirstChildElement("SubMeshIndex")->IntText()));
}