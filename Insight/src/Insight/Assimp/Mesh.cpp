#include "ispch.h"
#include "Mesh.h"
#include "Insight/Memory/MemoryManager.h"
#include "Insight/Instrumentor/Instrumentor.h"
#include "Insight/Renderer/Buffer.h"

Mesh::Mesh()
	: Insight::UUID()
	, m_created(false)
{ }

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures, 
	unsigned int subMeshIndex, const std::string& modelUUID, const std::string& meshName)
	: Insight::UUID()
	, m_created(false)
	, m_modelUUID(modelUUID)
	, m_meshName(meshName)
	, m_subMeshIndex(subMeshIndex)
{
	IS_PROFILE_FUNCTION();

	Create(vertices, indices, textures);
}

Mesh::~Mesh()
{
	IS_PROFILE_FUNCTION();

	DELETE_ON_HEAP(m_vertexBuffer);
	DELETE_ON_HEAP(m_indexBuffer);

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

		m_vertexBuffer = Insight::Render::VertexBuffer::Create(m_vertices);
		m_indexBuffer = Insight::Render::IndexBuffer::Create(indices);
	}
}

std::string& Mesh::GetName()
{
	return m_meshName;
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
