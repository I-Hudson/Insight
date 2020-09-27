#include "ispch.h"
#include "MeshComponent.h"

#include "Insight/Module/GraphicsModule.h"
#include "Insight/Library/ModelLibrary.h"
#include "Insight/Assimp/Model.h"

REGISTER_DEF_TYPE(MeshComponent);

MeshComponent::MeshComponent()
	: Component(nullptr)
	, m_mesh(nullptr)
	, m_materal(nullptr)
{
	m_updateEveryFarme = false;
	m_componentId = GetComponentID<MeshComponent>();
}

MeshComponent::MeshComponent(Entity* owner)
	: Component(owner)
	, m_mesh(nullptr)
	, m_materal(nullptr)
{
	m_updateEveryFarme = false;
	m_componentId = GetComponentID<MeshComponent>();
}

MeshComponent::~MeshComponent()
{
	SetMaterial(nullptr);
	Insight::Module::GraphicsModule::m_meshs.erase(std::find(Insight::Module::GraphicsModule::m_meshs.begin(), Insight::Module::GraphicsModule::m_meshs.end(), this));
}

void MeshComponent::OnCreate()
{
	Insight::Module::GraphicsModule::m_meshs.push_back(this);
	SetMaterial(Insight::Module::GraphicsModule::GetDefaultMaterial());
	m_updateEveryFarme = false;
}

float GetMin(float& f1, float& f2)
{
	return f1 < f2 ? f1 : f2;
}

float GetMax(float& f1, float& f2)
{
	return f1 > f2 ? f1 : f2;
}

void MeshComponent::SetMesh(Mesh* mesh)
{
	m_mesh = mesh;

	float xMin = 0;
	float xMax = 0;
	float yMin = 0;
	float yMax = 0;
	float zMin = 0;
	float zMax = 0;

	auto vertices = mesh->GetVertices();
	for (auto it = vertices.begin(); it != vertices.end(); ++it)
	{
		xMin = GetMin((*it).x, xMin);
		xMax = GetMax((*it).x, xMax);
		yMin = GetMin((*it).y, yMin);
		yMax = GetMax((*it).y, yMax);
		zMin = GetMin((*it).z, zMin);
		zMax = GetMax((*it).z, zMax);
	}
	m_boundingBox.Half_size.x = static_cast<float>(glm::abs(xMax - xMin) * 0.5);
	m_boundingBox.Half_size.y = static_cast<float>(glm::abs(yMax - yMin) * 0.5);
	m_boundingBox.Half_size.z = static_cast<float>(glm::abs(zMax - zMin) * 0.5);
}

MaterialRenderData MeshComponent::SetMaterial(Material* material)
{
	MaterialRenderData materialRenderData;

	if (m_materal != nullptr && m_materal != material)
	{
		m_materal->DecrementUsageCount(this);
		m_materal->m_isDirty = true;
	}

	m_materal = material;

	if (m_materal != nullptr)
	{
		materialRenderData = m_materal->IncrementUsageCount(this);
		m_materal->m_isDirty = true;
	}

	return materialRenderData;
}

void MeshComponent::Serialize(tinyxml2::XMLNode* out, tinyxml2::XMLDocument* doc, bool force)
{
	tinyxml2::XMLElement* Type = doc->NewElement("Type");
	Type->SetText("MeshComponent");
	out->InsertEndChild(Type);

	tinyxml2::XMLElement* MeshName = doc->NewElement("MeshName");
	MeshName->SetText(m_mesh->GetName().c_str());
	out->InsertEndChild(MeshName);

	tinyxml2::XMLElement* ModelUUID = doc->NewElement("ModelUUID");
	ModelUUID->SetText(m_mesh->GetModelUUID().c_str());
	out->InsertEndChild(ModelUUID);

	tinyxml2::XMLElement* SubMeshIndex = doc->NewElement("SubMeshIndex");
	SubMeshIndex->SetText(m_mesh->GetSubMeshIndex());
	out->InsertEndChild(SubMeshIndex);
}

void MeshComponent::Deserialize(tinyxml2::XMLNode* in, bool force)
{
	using namespace Insight::Library;
	SetMesh(ModelLibrary::GetInstance()->GetAsset(in->FirstChildElement("ModelUUID")->GetText())->GetSubMesh(in->FirstChildElement("SubMeshIndex")->IntText()));
}