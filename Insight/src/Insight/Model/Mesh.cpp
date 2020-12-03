#include "ispch.h"
#include "Mesh.h"
#include "Insight/Memory/MemoryManager.h"
#include "Insight/Instrumentor/Instrumentor.h"
#include "Insight/Renderer/Buffer.h"

#include "Platform/Vulkan/VulkanMaterial.h"
#include "Platform/Vulkan/VulkanDevice.h"
#include "Insight/Model/Model.h"

Mesh::Mesh()
	: Insight::UUID()
	, m_created(false)
{ }

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures, unsigned int subMeshIndex, Model* parentModel, const std::string & meshName)
	: Insight::UUID()
	, m_created(false)
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

	m_vertexBuffer->Destroy();
	m_indexBuffer->Destroy();

	DELETE_ON_HEAP(m_vertexBuffer);
	DELETE_ON_HEAP(m_indexBuffer);
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

	m_vertexBuffer = NEW_ON_HEAP(vks::VulkanBuffer);
	m_indexBuffer = NEW_ON_HEAP(vks::VulkanBuffer);

	vks::VulkanDevice::Instance()->CreateBufferGPU(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_vertices.size() * sizeof(Vertex), m_vertexBuffer, m_vertices.data());
	vks::VulkanDevice::Instance()->CreateBufferGPU(VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_indices.size() * sizeof(unsigned int), m_indexBuffer, m_indices.data());
}

void Mesh::Draw(VkCommandBuffer commandBuffer)
{
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &(m_vertexBuffer->buffer), offsets);
	vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer->buffer, 0, VK_INDEX_TYPE_UINT32);

	vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(m_indices.size()), 1, 0, 0, 0);
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
