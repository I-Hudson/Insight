#pragma once

#include "Insight/Core.h"
#include "Insight/UUID.h"

#include <vector>
#include <string>
#include <glm/glm.hpp>

struct Vertex
{
	glm::vec4 Position;
	glm::vec4 Colour;
	glm::vec4 Normal;
	glm::vec2 UV1;
};

struct Texture
{
	unsigned int ID;
	std::string Type;
	std::string Path;
};

class IS_API Mesh : public Insight::UUID
{
public:
	Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);
	~Mesh();

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

private:
	std::vector<Vertex> m_vertices;
	std::vector<unsigned int> m_indices;
	std::vector<Texture> m_textures;
};

