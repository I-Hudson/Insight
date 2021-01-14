#pragma once
#include "Insight/Core/Core.h"
#include "Insight/Component/Component.h"
#include "Insight/Model/Mesh.h"
#include "Insight/Renderer/Material.h"

#include <../vendor/glm/glm/glm.hpp>

class IS_API MeshComponent : public Component
{
public:
	MeshComponent();
	MeshComponent(SharedPtr<Entity> owner);
	virtual ~MeshComponent() override;

	virtual void OnCreate() override;
	virtual void OnDestroy() override;

	void SetMesh(WeakPtr<Mesh> mesh);
	WeakPtr<Mesh> GetMesh() const { return m_mesh; }

	void SetMaterial(WeakPtr<Material> material, int index);
	void SetMaterials(std::vector<WeakPtr<Material>> material, int index);

	std::string& GetMeshName() { return m_meshName; }

	virtual void Serialize(SharedPtr<Insight::Serialization::SerializableElement> element, bool force = false) override;
	virtual void Deserialize(SharedPtr<Insight::Serialization::SerializableElement> element, bool force = false) override;

private:
	WeakPtr<Mesh> m_mesh;
	std::vector<WeakPtr<Material>> m_materials;
	std::vector<MaterialBlockData> m_materialBlockDatas;

	std::string m_meshName;

	REGISTER_DEC_TYPE(MeshComponent);
};

