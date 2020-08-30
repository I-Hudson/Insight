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
#if defined(IS_VULKAN) && !defined(IS_OPENGL)
			return Memory::MemoryManager::NewOnFreeList<Platform::VulkanVertexBuffer>(vertices);
#elif defined(IS_OPENGL) && !defined(IS_VULKAN)
			IS_IMPLERMENT("Opengl vertex buffer missing.");
			return nullptr;
#else
			return nullptr;
#endif
		}

		IndexBuffer* IndexBuffer::Create(const std::vector<unsigned int>& indices)
		{
#if defined(IS_VULKAN) && !defined(IS_OPENGL)
			return Memory::MemoryManager::NewOnFreeList<Platform::VulkanIndexBuffer>(indices);
#elif defined(IS_OPENGL) && !defined(IS_VULKAN)
			IS_IMPLERMENT("Opengl index buffer missing.");
			return nullptr;
#else
			return nullptr;
#endif
		}
	}
}