#include "ispch.h"
#include "Engine/Model/SubMesh.h"
#include "Engine/Memory/MemoryManager.h"
#include "Engine/Instrumentor/Instrumentor.h"
#include "Engine/Graphics/GPUBuffer.h"

#include "Platform/Vulkan/VulkanMaterial.h"
#include "Platform/Vulkan/VulkanDevice.h"
#include "Engine/Model/Model.h"

SubMesh::SubMesh()
	: m_created(false)
{
	m_vertexBuffer = Insight::Graphics::GPUBuffer::New();
	m_indexBuffer = Insight::Graphics::GPUBuffer::New();
}

SubMesh::SubMesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices)
	: m_created(false)
{
	IS_PROFILE_FUNCTION();

	m_vertexBuffer = Insight::Graphics::GPUBuffer::New();
	m_indexBuffer = Insight::Graphics::GPUBuffer::New();

	Create(vertices, indices);
}

SubMesh::SubMesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices, const LoadedTextureStrings& loadedTextureStrings)
	: m_created(false)
{
	IS_PROFILE_FUNCTION();

	m_vertexBuffer = Insight::Graphics::GPUBuffer::New();
	m_indexBuffer = Insight::Graphics::GPUBuffer::New();
	m_loadedTexturesStrings = loadedTextureStrings;

	Create(vertices, indices);
}

SubMesh::~SubMesh()
{
	IS_PROFILE_FUNCTION();

	m_vertices.clear();
	m_indices.clear();

	m_vertexBuffer->ReleaseGPU();
	m_indexBuffer->ReleaseGPU();

	::Delete(m_vertexBuffer);
	::Delete(m_indexBuffer);
}

void SubMesh::Create(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices)
{
	IS_PROFILE_FUNCTION();

	if (!m_created)
	{
		m_created = true;

		m_vertices = vertices;
		m_indices = indices;
	}

	m_vertexBuffer->Init(Insight::Graphics::GPUBufferDesc::Vertex(sizeof(Vertex), m_vertices.size(), m_vertices.data()));
	m_indexBuffer->Init(Insight::Graphics::GPUBufferDesc::Index(sizeof(u32), m_indices.size(), m_indices.data()));
}

//void SubMesh::Draw(VkCommandBuffer commandBuffer)
//{
//	VkDeviceSize offsets[] = { 0 };
//	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &(m_vertexBuffer->buffer), offsets);
//	vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer->buffer, 0, VK_INDEX_TYPE_UINT32);
//
//	vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(m_indices.size()), 1, 0, 0, 0);
//}

void SubMesh::SetVertices(const std::vector<Vertex>& vertices)
{
	m_vertices = vertices;
}

void SubMesh::SetIndices(const std::vector<unsigned int>& indices)
{
	m_indices = indices;
}

void SubMesh::Rebuild()
{
	m_vertexBuffer->Init(Insight::Graphics::GPUBufferDesc::Vertex(sizeof(Vertex), m_vertices.size(), m_vertices.data()));
	m_indexBuffer->Init(Insight::Graphics::GPUBufferDesc::Index(sizeof(u32), m_indices.size(), m_indices.data()));
}

std::vector<Vertex> SubMesh::GetVertices()
{
	return m_vertices;
}

std::vector<glm::vec3> SubMesh::GetColours()
{
	std::vector<glm::vec3> colours;
	for (auto it = m_vertices.begin(); it != m_vertices.end(); ++it)
	{
		colours.push_back((*it).Colour);
	}
	return colours;
}

std::vector<glm::vec3> SubMesh::GetNormals()
{
	std::vector<glm::vec3> normals;
	for (auto it = m_vertices.begin(); it != m_vertices.end(); ++it)
	{
		normals.push_back((*it).Normal);
	}
	return normals;
}

std::vector<unsigned int> SubMesh::GetIndices()
{
	return m_indices;
}

std::vector<glm::vec2> SubMesh::GetUVs()
{
	std::vector<glm::vec2> uvs;
	for (auto it = m_vertices.begin(); it != m_vertices.end(); ++it)
	{
		uvs.push_back((*it).UV1);
	}
	return uvs;
}
