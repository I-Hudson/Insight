#include "ispch.h"
#include "Insight/Renderer/Buffer.h"
#include "Insight/Memory/MemoryManager.h"

#ifdef IS_VULKAN
#include "Insight/Renderer/Vulkan/VulkanBuffers.h"
typedef Insight::Render::VulkanVertexBuffer PlatformVertexBuffer;
typedef Insight::Render::VulkanIndexBuffer PlatformIndexBuffer;
#elif defined(IS_OPENGL)
#include "Insight/Renderer/OpenGL/OpenGLBuffers.h"
typedef Insight::Render::OpenGLVertexBuffer PlatformVertexBuffer;
typedef Insight::Render::OpenGLIndexBuffer PlatformIndexBuffer;
#endif


namespace Insight
{
	namespace Render
	{
		VertexBuffer* VertexBuffer::Create(const std::vector<Vertex>& vertices)
		{
			return Memory::MemoryManager::NewOnFreeList<PlatformVertexBuffer>(vertices);
		}

		IndexBuffer* IndexBuffer::Create(const std::vector<unsigned int>& indices)
		{
			return Memory::MemoryManager::NewOnFreeList<PlatformIndexBuffer>(indices);
		}
	}
}