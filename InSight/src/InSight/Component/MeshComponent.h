#pragma once
#include "Insight/Core.h"
#include "Insight/Component/Component.h"
#include "Insight/Assimp/Mesh.h"
#include "Insight/Renderer/Material.h"

#include <../vendor/glm/glm/glm.hpp>

class IS_API MeshComponent : public Component
{
public:
	MeshComponent();
	MeshComponent(Entity* owner);
	virtual ~MeshComponent() override;

	void SetMesh(Mesh* mesh);
	Mesh* GetMesh() const { return m_mesh; }

	void SetMaterial(Material* material);
	Material* GeMaterial() const { return m_materal; }

	virtual void Serialize(json& out, bool force = false) override;
	virtual void Deserialize(json in, bool force = false) override;

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

	struct BoundingBox;
	BoundingBox GetBoundingBox() { return m_boundingBox; }

private:
	// define the operations to be used in our 3D vertices
	struct BBvec3
	{
		float x, y, z;
		BBvec3 operator- (const BBvec3& rhs) const { return{ x - rhs.x, y - rhs.y, z - rhs.z }; }
		float operator* (const BBvec3& rhs) const { return{ x * rhs.x + y * rhs.y + z * rhs.z }; } // DOT PRODUCT
		BBvec3 operator^ (const BBvec3& rhs) const { return{ y * rhs.z - z * rhs.y, z * rhs.x - x * rhs.z, x * rhs.y - y * rhs.x }; } // CROSS PRODUCT
		BBvec3 operator* (const float& rhs)const { return BBvec3{ x * rhs, y * rhs, z * rhs }; }
	};

	struct BoundingBox
	{
		BBvec3 Pos, AxisX, AxisY, AxisZ, Half_size;

		// check if there's a separating plane in between the selected axes
		bool GetSeparatingPlane(const BBvec3& RPos, const BBvec3& Plane, const BoundingBox& box1, const BoundingBox& box2)
		{
			return (fabs(RPos * Plane) >
				(fabs((box1.AxisX * box1.Half_size.x) * Plane) +
					fabs((box1.AxisY * box1.Half_size.y) * Plane) +
					fabs((box1.AxisZ * box1.Half_size.z) * Plane) +
					fabs((box2.AxisX * box2.Half_size.x) * Plane) +
					fabs((box2.AxisY * box2.Half_size.y) * Plane) +
					fabs((box2.AxisZ * box2.Half_size.z) * Plane)));
		}

		// test for separating planes in all 15 axes
		bool GetCollision(const BoundingBox& box1, const BoundingBox& box2)
		{
			static BBvec3 RPos;
			RPos = box2.Pos - box1.Pos;

			return !(GetSeparatingPlane(RPos, box1.AxisX, box1, box2) ||
				GetSeparatingPlane(RPos, box1.AxisY, box1, box2) ||
				GetSeparatingPlane(RPos, box1.AxisZ, box1, box2) ||
				GetSeparatingPlane(RPos, box2.AxisX, box1, box2) ||
				GetSeparatingPlane(RPos, box2.AxisY, box1, box2) ||
				GetSeparatingPlane(RPos, box2.AxisZ, box1, box2) ||
				GetSeparatingPlane(RPos, box1.AxisX ^ box2.AxisX, box1, box2) ||
				GetSeparatingPlane(RPos, box1.AxisX ^ box2.AxisY, box1, box2) ||
				GetSeparatingPlane(RPos, box1.AxisX ^ box2.AxisZ, box1, box2) ||
				GetSeparatingPlane(RPos, box1.AxisY ^ box2.AxisX, box1, box2) ||
				GetSeparatingPlane(RPos, box1.AxisY ^ box2.AxisY, box1, box2) ||
				GetSeparatingPlane(RPos, box1.AxisY ^ box2.AxisZ, box1, box2) ||
				GetSeparatingPlane(RPos, box1.AxisZ ^ box2.AxisX, box1, box2) ||
				GetSeparatingPlane(RPos, box1.AxisZ ^ box2.AxisY, box1, box2) ||
				GetSeparatingPlane(RPos, box1.AxisZ ^ box2.AxisZ, box1, box2));
		}
	};

	BoundingBox m_boundingBox;
	Mesh* m_mesh;
	Material* m_materal;

	REGISTER_DEC_TYPE(MeshComponent);
};

