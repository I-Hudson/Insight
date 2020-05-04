#include "ispch.h"
#include "Mesh.h"

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures)
	: Insight::UUID()
{
	m_vertices = vertices;
	m_indices = indices;
	m_textures = textures;
}

Mesh::~Mesh()
{

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
