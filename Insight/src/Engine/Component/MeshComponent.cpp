#include "ispch.h"
#include "Engine/Component/MeshComponent.h"

#include "Engine/Module/GraphicsModule.h"
#include "Engine/Library/ModelLibrary.h"
#include "Engine/Model/Model.h"
#include "Engine/Graphics/RenderList.h"

REGISTER_DEF_TYPE(MeshComponent);

MeshComponent::MeshComponent(ComponentManager* componentManager, ComponentID componentID, ComponentType componentType, EntityManager* entityManager, EntityID entity)
	: Component(componentManager, componentID, componentType, entityManager, entity)
	, m_mesh(nullptr)
	, m_meshName("")
{
	SetType<MeshComponent>();
	//m_updateEveryFarme = false;
}

MeshComponent::~MeshComponent()
{
}

//void MeshComponent::OnCreate()
//{
//	__super::OnCreate();
//
//	IS_PROPERTY(std::string, m_meshName, "Mesh Name", ShowInEditor | ReadOnly);
//
//	Module::GraphicsModule::m_meshs.push_back(this);
//	//SetMaterial(Module::GraphicsModule::GetDefaultMaterial());
//	m_updateEveryFarme = false;
//}

//void MeshComponent::OnDestroy()
//{
//	auto it = std::find_if(Module::GraphicsModule::m_meshs.begin(), Module::GraphicsModule::m_meshs.end(), [&](MeshComponent* component)
//		{
//			if (component)
//			{
//				return this == component;
//			}
//			return false;
//		});
//	Module::GraphicsModule::m_meshs.erase(it);
//}

//void MeshComponent::Draw(VkCommandBuffer cmd, MeshMaterialUpdateFunc materialUpdateFunc)
//{
//	if (m_mesh)
//	{
//		m_mesh->Draw(cmd, m_materials, m_materialBlockDatas, materialUpdateFunc, this);
//	}
//}

void MeshComponent::SetModel(Insight::Model* model)
{
	if (model)
	{
		Insight::Mesh* mesh = &const_cast<Insight::Mesh&>(model->GetMesh());
		SetMesh(mesh);
		//SetMaterials(model->GetMaterals());
	}
}

void MeshComponent::SetMesh(Insight::Mesh* mesh)
{
	m_mesh = mesh;
}

//void MeshComponent::SetMaterial(Material* material, int index)
//{
//	if (index < 0 || index > m_materials.size())
//	{
//		IS_CORE_ERROR("[MeshComponent::SetMaterial] Index is not within Materials bounds.");
//		return;
//	}
//	m_materials[index] = material;
//	m_materialBlockDatas[index] = MaterialBlockData();
//}

//void MeshComponent::SetMaterials(std::vector<Material*> materials)
//{
//	m_materials = materials;
//	m_materialBlockDatas.clear();
//	m_materialBlockDatas.resize(m_materials.size());
//}

//void MeshComponent::SetMaterialBlockData(const std::vector<MaterialBlockData>& materialBlockDatas)
//{
//	m_materialBlockDatas = materialBlockDatas;
//}

//void MeshComponent::Serialize(Serialization::SerializableElement* element, bool force)
//{
//	element->AddAttribute("UUID", GetUUID());
//	if (m_mesh)
//	{
//		//element->AddAttribute("MeshName", meshSP->GetName().c_str());
//	}
//}
//
//void MeshComponent::Deserialize(Serialization::SerializableElement* element, bool force)
//{
//	//SetMesh(ModelLibrary::Instance()->GetAsset(in->FirstChildElement("ModelUUID")->GetText())->GetSubMesh(in->FirstChildElement("SubMeshIndex")->IntText()));
//}

void MeshComponent::OnUpdate(const float& a_deltaTime)
{
}

void MeshComponent::OnDraw(Insight::Graphics::RenderListView* renderList, const glm::mat4& worldTransform, const Insight::Maths::Frustum& cameraFrustum)
{
	using namespace Insight::Graphics;
	m_mesh->Draw(renderList, worldTransform, cameraFrustum);
}
