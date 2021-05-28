#pragma once

#include "Engine/Component/Component.h"

#include "Reflect.h"
#include "Generated/MeshComponent_reflect_generated.h"

#include <../vendor/glm/glm/glm.hpp>

namespace Insight::Graphics
{
	class Model;
	class Mesh;
}

REFLECT_CLASS()
class MeshComponent : public Component
{
	REFLECT_GENERATED_BODY()

public:
	MeshComponent() { }
	MeshComponent(ComponentManager* componentManager, ComponentID componentID, ComponentType componentType, EntityManager* entityManager, EntityID entity);
	virtual ~MeshComponent() override;

	virtual void OnUpdate(const float& a_deltaTime) override;
	virtual void OnDraw(Insight::Graphics::RenderList* renderList) override;
	//virtual void OnCreate() override;
	//virtual void OnDestroy() override;

	void SetModel(Insight::Graphics::Model* model);
	void SetMesh(Insight::Graphics::Mesh* mesh);
	Insight::Graphics::Mesh* GetMesh() const { return m_mesh; }

	//void SetMaterial(Material* material, int index);
	//void SetMaterials(std::vector<Material*> materials);

	//std::vector<Material*>& GetMaterials() { return m_materials; }
	//std::vector<MaterialBlockData>& GetMaterialBlockData() { return m_materialBlockDatas; }
	//void SetMaterialBlockData(const std::vector<MaterialBlockData>& materialBlockDatas);
	std::string& GetMeshName() { return m_meshName; }

	//virtual void Serialize(Serialization::SerializableElement* element, bool force = false) override;
	//virtual void Deserialize(Serialization::SerializableElement* element, bool force = false) override;

private:
	Insight::Graphics::Mesh* m_mesh;

	//std::vector<Material*> m_materials;
	//std::vector<MaterialBlockData> m_materialBlockDatas;


	REFLECT_PROPERTY(ShowInEditor)
	std::string m_meshName;

	REGISTER_DEC_TYPE(MeshComponent);
};

