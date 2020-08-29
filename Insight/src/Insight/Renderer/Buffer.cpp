#include "ispch.h"
#include "Insight/Renderer/Buffer.h"
#include "Insight/Memory/MemoryManager.h"

#include "Platform/Vulkan/VulkanBuffers.h"
#include "Platform/OpenGL/OpenGLBuffers.h"


namespace Insight
{
	namespace Render
	{
		VertexBuffer* VertexBuffer::Create(const std::vector<Vertex>& vertices)
		{
			return Memory::MemoryManager::NewOnFreeList<Platform::VulkanVertexBuffer>(vertices);
		}

		IndexBuffer* IndexBuffer::Create(const std::vector<unsigned int>& indices)
		{
			return Memory::MemoryManager::NewOnFreeList<Platform::VulkanIndexBuffer>(indices);
		}
	}
}