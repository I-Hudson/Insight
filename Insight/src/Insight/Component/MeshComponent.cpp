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

	Module::GraphicsModule::m_meshs.push_back(shared_from_this());
	//SetMaterial(Module::GraphicsModule::GetDefaultMaterial());
	m_updateEveryFarme = false;
}

void MeshComponent::OnDestroy()
{
	auto it = std::find_if(Module::GraphicsModule::m_meshs.begin(), Module::GraphicsModule::m_meshs.end(), [&](WeakPtr<MeshComponent> component)
		{
			if (auto comSP = component.lock())
			{
				return this == comSP.get();
			}
			return false;
		});
	Module::GraphicsModule::m_meshs.erase(it);
}

void MeshComponent::Draw(VkCommandBuffer cmd, MeshMaterialUpdateFunc materialUpdateFunc)
{
	if (auto meshSP = m_mesh.lock())
	{
		meshSP->Draw(cmd, m_materials, m_materialBlockDatas, materialUpdateFunc, this);
	}
}

void MeshComponent::SetMesh(WeakPtr<Mesh> mesh)
{
	m_mesh = mesh;
	m_meshName = mesh.lock()->GetMeshName();
}

void MeshComponent::SetModel(WeakPtr<Model> model)
{
	if (auto modelSP = model.lock())
	{
		SetMesh(modelSP->GetMesh());
		SetMaterials(modelSP->GetMaterals());
	}
}

void MeshComponent::SetMaterial(WeakPtr<Material> material, int index)
{
	if (index < 0 || index > m_materials.size())
	{
		IS_CORE_ERROR("[MeshComponent::SetMaterial] Index is not within Materials bounds.");
		return;
	}
	m_materials[index] = material;
	m_materialBlockDatas[index] = MaterialBlockData();
}

void MeshComponent::SetMaterials(std::vector<WeakPtr<Material>> materials)
{
	m_materials = materials;
	m_materialBlockDatas.clear();
	m_materialBlockDatas.resize(m_materials.size());
}

void MeshComponent::SetMaterialBlockData(const std::vector<MaterialBlockData>& materialBlockDatas)
{
	m_materialBlockDatas = materialBlockDatas;
}

void MeshComponent::Serialize(SharedPtr<Serialization::SerializableElement> element, bool force)
{
	element->AddAttribute("UUID", GetUUID());
	if (auto meshSP = m_mesh.lock())
	{
		//element->AddAttribute("MeshName", meshSP->GetName().c_str());
	}
}

void MeshComponent::Deserialize(SharedPtr<Serialization::SerializableElement> element, bool force)
{
	//SetMesh(ModelLibrary::Instance()->GetAsset(in->FirstChildElement("ModelUUID")->GetText())->GetSubMesh(in->FirstChildElement("SubMeshIndex")->IntText()));
}