#include "ispch.h"
#include "SubMesh.h"
#include "Insight/Memory/MemoryManager.h"
#include "Insight/Instrumentor/Instrumentor.h"
#include "Insight/Renderer/Buffer.h"

#include "Platform/Vulkan/VulkanMaterial.h"
#include "Platform/Vulkan/VulkanDevice.h"
#include "Insight/Model/Model.h"

SubMesh::SubMesh()
	: Insight::UUID()
	, m_created(false)
{ }

SubMesh::SubMesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices)
	: Insight::UUID()
	, m_created(false)
{
	IS_PROFILE_FUNCTION();

	Create(vertices, indices);
}

SubMesh::~SubMesh()
{
	IS_PROFILE_FUNCTION();

	m_vertices.clear();
	m_indices.clear();

	m_vertexBuffer->Destroy();
	m_indexBuffer->Destroy();

	m_vertexBuffer.reset();
	m_indexBuffer.reset();
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

	m_vertexBuffer = Insight::Object::CreateObject<vks::VulkanBuffer>();
	m_indexBuffer = Insight::Object::CreateObject<vks::VulkanBuffer>();

	vks::VulkanDevice::Instance()->CreateBufferGPU(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_vertices.size() * sizeof(Vertex), m_vertexBuffer.get(), m_vertices.data());
	vks::VulkanDevice::Instance()->CreateBufferGPU(VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_indices.size() * sizeof(unsigned int), m_indexBuffer.get(), m_indices.data());
}

void SubMesh::Draw(VkCommandBuffer commandBuffer)
{
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &(m_vertexBuffer->buffer), offsets);
	vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer->buffer, 0, VK_INDEX_TYPE_UINT32);

	vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(m_indices.size()), 1, 0, 0, 0);
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
