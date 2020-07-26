#include "ispch.h"
#include "Mesh.h"
#include "Insight/Memory/MemoryManager.h"

Mesh::Mesh()
	: Insight::UUID()
	, m_created(false)
{ }

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures)
	: Insight::UUID()
	, m_created(false)
{
	Create(vertices, indices, textures);
}

Mesh::~Mesh()
{
	Insight::Memory::MemoryManager::DeleteOnFreeList(m_vertexBuffer);
	Insight::Memory::MemoryManager::DeleteOnFreeList(m_indexBuffer);

	m_vertices.clear();
	m_indices.clear();
	m_textures.clear();
}

void Mesh::Create(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures)
{
	if (!m_created)
	{
		m_created = true;

		m_vertices = vertices;
		m_indices = indices;
		m_textures = textures;

		m_vertexBuffer = VertexBuffer::Create(m_vertices);
		m_indexBuffer = IndexBuffer::Create(indices);
	}
}

std::vector<glm::vec3> Mesh::GetVertices()
{
	std::vector<glm::vec3> vertices;
	for (auto it = m_vertices.begin(); it != m_vertices.end(); ++it)
	{
		vertices.push_back((*it).Position);
	}
	return vertices;
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
