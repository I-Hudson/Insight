#pragma once
#include "Insight/Core.h"
#include "Insight/Component/Component.h"
#include "Insight/Assimp/Mesh.h"
#include "Insight/Renderer/Material.h"

#include <../vendor/glm/glm/glm.hpp>

class IS_API MeshComponent : public Component
{
public:
	MeshComponent(Entity* owner);
	virtual ~MeshComponent() override;

	void SetMesh(Mesh* mesh) { m_mesh = mesh; }
	Mesh* GetMesh() const { return m_mesh; }

	void SetMaterial(Material* material) { m_materal = material; }
	Material* GeMaterial() const { return m_materal; }

	// Returns a new array of vertices.
	std::vector<glm::vec3> GetVertices() const { return m_mesh->GetVertices(); }
	// Returns a new array of colours.
	std::vector<glm::vec3> GetColours() const { return m_mesh->GetColours(); }
	// Returns a new array of normals.
	std::vector<glm::vec3> GetNormals() const { return m_mesh->GetNormals(); }
	// Returns a new array of indcies.
	std::vector<unsigned int> GetIndices() const { return m_mesh->GetIndices(); }
	// Returns a new array of uvs.
	std::vector<glm::vec2> GetUVs() const { return m_mesh->GetUVs(); }

private:
	Mesh* m_mesh;
	Material* m_materal;
};

