#include "ispch.h"
#include "Insight/Renderer/Buffer.h"

#include "Insight/Renderer/VulkanBuffers.h"
#include "Insight/Memory/MemoryManager.h"

namespace Insight
{
	namespace Render
	{
		VertexBuffer* VertexBuffer::Create(const std::vector<Vertex>& vertices)
		{
			return Memory::MemoryManager::NewOnFreeList<VulkanVertexBuffer>(vertices);
		}

		IndexBuffer* IndexBuffer::Create(const std::vector<unsigned int>& indices)
		{
			return Memory::MemoryManager::NewOnFreeList<VulkanIndexBuffer>(indices);
		}
	}
}