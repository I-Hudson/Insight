#include "ispch.h"
#include "MeshComponent.h"

#include "Insight/Module/GraphicsModule.h"
#include "Insight/Library/ModelLibrary.h"
#include "Insight/Assimp/Model.h"

REGISTER_DEF_TYPE(MeshComponent);

MeshComponent::MeshComponent()
	: Component(nullptr, ComponentType::MESH)
	, m_mesh(nullptr)
	, m_materal(nullptr)
{
	Insight::Module::GraphicsModule::m_meshs.push_back(this);
	SetMaterial(Insight::Module::GraphicsModule::GetDefaultMaterial());
}

MeshComponent::MeshComponent(Entity* owner)
	: Component(owner, ComponentType::MESH)
	, m_mesh(nullptr)
	, m_materal(nullptr)
{
	Insight::Module::GraphicsModule::m_meshs.push_back(this);
	SetMaterial(Insight::Module::GraphicsModule::GetDefaultMaterial());
}

MeshComponent::~MeshComponent()
{
	Insight::Module::GraphicsModule::m_meshs.erase(std::find(Insight::Module::GraphicsModule::m_meshs.begin(), Insight::Module::GraphicsModule::m_meshs.end(), this));
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
	m_boundingBox.Half_size.x = glm::abs(xMax - xMin) * 0.5;
	m_boundingBox.Half_size.y = glm::abs(yMax - yMin) * 0.5;
	m_boundingBox.Half_size.z = glm::abs(zMax - zMin) * 0.5;
}

void MeshComponent::SetMaterial(Material* material)
{
	if (m_materal != nullptr && m_materal != material)
	{
		m_materal->DecrementUsageCount();
		m_materal->m_isDirty = true;
	}

	m_materal = material;

	if (m_materal != nullptr)
	{
		m_materal->IncrementUsageCount();
		m_materal->m_isDirty = true;

	}
}

void MeshComponent::Serialize(json& out, bool force)
{
	out["UUID"] = GetUUID();
	out["Type"] = "MeshComponent";
	out["MeshName"] = m_mesh->GetName();
	out["ModelUUID"] = m_mesh->GetModelUUID();
	out["SubMeshIndex"] = m_mesh->GetSubMeshIndex();
}

void MeshComponent::Deserialize(json in, bool force)
{
	using namespace Insight::Library;

	SetUUID(in["UUID"]);
	SetMesh(ModelLibrary::GetInstance()->GetAsset(in["ModelUUID"])->GetSubMesh(in["SubMeshIndex"]));
}