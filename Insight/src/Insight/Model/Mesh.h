#pragma once

#include "Insight/Core/Core.h"
#include "Platform/Vulkan/VulkanHeader.h"
#include "Insight/Core/UUID.h"
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

namespace vks
{
	class VulkanBuffer;
}

class Model;

class IS_API Mesh : public Insight::UUID
{
public:
	Mesh();
	Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures, unsigned int subMeshIndex, Model* modelUUID, const std::string& meshName);
	~Mesh();

	void Create(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);

	void Draw(VkCommandBuffer commandBuffer);

	std::string& GetName();
	const std::string GetModelUUID() const;
	const unsigned int GetSubMeshIndex() const { return m_subMeshIndex; }

	unsigned int GetVertexCount() { return static_cast<unsigned int>(m_vertices.size()); }
	unsigned int GetIndicesCount() { return static_cast<unsigned int>(m_indices.size()); }

	// Returns a new array of vertices.
	std::vector<Vertex> GetVertices();
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

private:
	std::vector<Vertex> m_vertices;
	std::vector<unsigned int> m_indices;
	std::vector<Texture> m_textures;

	SharedPtr<vks::VulkanBuffer> m_vertexBuffer;
	SharedPtr<vks::VulkanBuffer> m_indexBuffer;

	Model* m_parentModel;
	std::string m_meshName;
	unsigned int m_subMeshIndex;
	bool m_created;
};

