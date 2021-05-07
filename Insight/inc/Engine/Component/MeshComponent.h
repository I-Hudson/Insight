#pragma once

#include "Engine/Component/Component.h"
#include "Engine/Model/Mesh.h"
#include "Engine/Graphics/Material.h"

#include "Reflect.h"
#include "Generated/MeshComponent_reflect_generated.h"

#include <../vendor/glm/glm/glm.hpp>

IS_API REFLECT_CLASS(MeshComponent), public Component
{
	REFLECT_GENERATED_BODY()

public:
	MeshComponent();
	MeshComponent(Entity* owner);
	virtual ~MeshComponent() override;

	virtual void OnCreate() override;
	virtual void OnDestroy() override;

	void SetMesh(Mesh* mesh);
	void SetModel(Model* model);
	Mesh* GetMesh() const { return m_mesh; }

	void SetMaterial(Material* material, int index);
	void SetMaterials(std::vector<Material*> materials);

	std::vector<Material*>& GetMaterials() { return m_materials; }
	std::vector<MaterialBlockData>& GetMaterialBlockData() { return m_materialBlockDatas; }
	void SetMaterialBlockData(const std::vector<MaterialBlockData>& materialBlockDatas);
	std::string& GetMeshName() { return m_meshName; }

	virtual void Serialize(Serialization::SerializableElement* element, bool force = false) override;
	virtual void Deserialize(Serialization::SerializableElement* element, bool force = false) override;

private:
	Mesh* m_mesh;
	std::vector<Material*> m_materials;
	std::vector<MaterialBlockData> m_materialBlockDatas;

	REFLECT_PROPERTY()
	std::string m_meshName;

	REGISTER_DEC_TYPE(MeshComponent);
};

