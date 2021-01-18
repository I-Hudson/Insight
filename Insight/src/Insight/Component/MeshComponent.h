#pragma once
#include "Insight/Core/Core.h"
#include "Insight/Component/Component.h"
#include "Insight/Model/Mesh.h"
#include "Insight/Renderer/Material.h"

#include <../vendor/glm/glm/glm.hpp>

class IS_API MeshComponent : public Component
	, public std::enable_shared_from_this<MeshComponent>
{
public:
	MeshComponent();
	MeshComponent(SharedPtr<Entity> owner);
	virtual ~MeshComponent() override;

	virtual void OnCreate() override;
	virtual void OnDestroy() override;

	void Draw(VkCommandBuffer cmd);

	void SetMesh(WeakPtr<Mesh> mesh);
	void SetModel(WeakPtr<Model> model);
	WeakPtr<Mesh> GetMesh() const { return m_mesh; }

	void SetMaterial(WeakPtr<Material> material, int index);
	void SetMaterials(std::vector<WeakPtr<Material>> materials);

	std::vector<WeakPtr<Material>>& GetMaterails() { return m_materials; }
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

