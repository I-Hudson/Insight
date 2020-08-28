#pragma once

#include "Insight/Core.h"
#include "Insight/UUID.h"
#include "Insight/Renderer/Buffer.h"

#include <vector>
#include <string>
#include <../vendor/glm/glm/glm.hpp>

struct Texture
{
	unsigned int ID;
	std::string Type;
	std::string Path;
};

namespace Platform
{
	class VulkanRenderer;
	class Swapchain;
}

class IS_API Mesh : public Insight::UUID
{
public:
	Mesh();
	Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures,
		unsigned int subMeshIndex, const std::string& modelUUID, const std::string& meshName);
	~Mesh();

	void Create(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);

	std::string& GetName();
	const std::string& GetModelUUID() const { return m_modelUUID; }
	const unsigned int GetSubMeshIndex() const { return m_subMeshIndex; }

	unsigned int GetVertexCount() { return static_cast<unsigned int>(m_vertices.size()); }
	unsigned int GetIndicesCount() { return static_cast<unsigned int>(m_indices.size()); }

	// Returns a new array of vertices.
	std::vector<glm::vec3> GetVertices();
	// Returns a new array of colours.
	std::vector<glm::vec3> GetColours();
	// Returns a new array of normals.
	std::vector<glm::vec3> GetNormals();
	// Returns a new array of indcies.
	std::vector<unsigned int> GetIndices();
	// Returns a new array of uvs.
	std::vector<glm::vec2> GetUVs();

	std::vector<Texture> GetTextures();

private:
	Insight::Render::VertexBuffer* GetVertexBuffer() { return m_vertexBuffer; }
	Insight::Render::IndexBuffer* GetIndexBuffer() { return m_indexBuffer; }

private:
	std::vector<Vertex> m_vertices;
	std::vector<unsigned int> m_indices;
	std::vector<Texture> m_textures;

	Insight::Render::VertexBuffer* m_vertexBuffer;
	Insight::Render::IndexBuffer* m_indexBuffer;

	std::string m_modelUUID;
	std::string m_meshName;
	unsigned int m_subMeshIndex;
	bool m_created;

	friend Platform::VulkanRenderer;
	friend Platform::Swapchain;
};

