#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Graphics/GPUBuffer.h"
#include "Platform/Vulkan/VulkanHeader.h"

#include <vector>
#include <string>
#include <../vendor/glm/glm/glm.hpp>

namespace vks
{
	class VulkanBuffer;
}

class Model;

using LoadedTextureStrings = std::vector<std::pair<std::string, std::string>>;

class IS_API SubMesh
{
public:

	SubMesh();
	SubMesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
	SubMesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices, const LoadedTextureStrings& loadedTextureStrings);
	~SubMesh();

	void Create(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);

	unsigned int GetVertexCount() { return static_cast<unsigned int>(m_vertices.size()); }
	unsigned int GetIndicesCount() { return static_cast<unsigned int>(m_indices.size()); }

	void SetVertices(const std::vector<Vertex>& vertices);
	void SetIndices(const std::vector<unsigned int>& indices);
	void Rebuild();

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

	Insight::Graphics::GPUBuffer* GetGPUVerticesBuffer() { return m_vertexBuffer; }
	Insight::Graphics::GPUBuffer* GetGPUIndexBuffer() { return m_indexBuffer; }

	std::string GetDiffuseTextureString()
	{
		if (m_loadedTexturesStrings.size() == 0)
		{
			return "";
		}
		return m_loadedTexturesStrings[0].second;
	}

private:
	std::vector<Vertex> m_vertices;
	std::vector<unsigned int> m_indices;
	LoadedTextureStrings m_loadedTexturesStrings;

	Insight::Graphics::GPUBuffer* m_vertexBuffer;
	Insight::Graphics::GPUBuffer* m_indexBuffer;
	bool m_created;
};

