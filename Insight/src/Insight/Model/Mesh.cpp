#include "ispch.h"
#include "Mesh.h"
#include "Insight/Memory/MemoryManager.h"
#include "Insight/Instrumentor/Instrumentor.h"
#include "Insight/Renderer/Buffer.h"

#include "Platform/Vulkan/VulkanMaterial.h"
#include "Insight/Model/Model.h"

Mesh::Mesh()
	: Insight::UUID()
	, m_created(false)
{ }

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures, U16 firstIndex, U16 indexCount, unsigned int subMeshIndex, Model* parentModel, const std::string & meshName)
	: Insight::UUID()
	, m_created(false)
	, m_firstIndex(firstIndex)
	, m_indexCount(indexCount)
	, m_parentModel(parentModel)
	, m_meshName(meshName)
	, m_subMeshIndex(subMeshIndex)
{
	IS_PROFILE_FUNCTION();

	Create(vertices, indices, textures);
}

Mesh::~Mesh()
{
	IS_PROFILE_FUNCTION();

	m_vertices.clear();
	m_indices.clear();
	m_textures.clear();
}

void Mesh::Create(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures)
{
	IS_PROFILE_FUNCTION();

	if (!m_created)
	{
		m_created = true;

		m_vertices = vertices;
		m_indices = indices;
		m_textures = textures;
	}
}

void Mesh::Draw(VkCommandBuffer commandBuffer)
{
	vkCmdDrawIndexed(commandBuffer, m_indexCount, 1, m_firstIndex, 0, 0);
}

std::string& Mesh::GetName()
{
	return m_meshName;
}

const std::string& Mesh::GetModelUUID() const
{
	if (m_parentModel)
	{
		return m_parentModel->GetUUID();
	}
	return "INVALID";
}

std::vector<Vertex> Mesh::GetVertices()
{
	return m_vertices;
}

std::vector<glm::vec3> Mesh::GetColours()
{
	std::vector<glm::vec3> colours;
	for (auto it = m_vertices.begin(); it != m_vertices.end(); ++it)
	{
		colours.push_back((*it).Colour);
	}
	return colours;
}

std::vector<glm::vec3> Mesh::GetNormals()
{
	std::vector<glm::vec3> normals;
	for (auto it = m_vertices.begin(); it != m_vertices.end(); ++it)
	{
		normals.push_back((*it).Normal);
	}
	return normals;
}

std::vector<unsigned int> Mesh::GetIndices()
{
	return m_indices;
}

std::vector<glm::vec2> Mesh::GetUVs()
{
	std::vector<glm::vec2> uvs;
	for (auto it = m_vertices.begin(); it != m_vertices.end(); ++it)
	{
		uvs.push_back((*it).UV1);
	}
	return uvs;
}

std::vector<Texture> Mesh::GetTextures()
{
	return m_textures;
}
