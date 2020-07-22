#include "ispch.h"
#ifdef IS_OPENGL
#include "Insight/Renderer/OpenGL/OpenGL.h"
#include "Insight/Renderer/OpenGL/OpenGLBuffers.h"

namespace Insight
{
	namespace Render
	{
		OpenGLRenderer* OpenGLVertexBuffer::s_Renderer;
		OpenGLRenderer* OpenGLIndexBuffer::s_Renderer;

		
		OpenGLVertexBuffer::OpenGLVertexBuffer(const std::vector<Vertex>& vertices)
		{
		}

		OpenGLVertexBuffer::~OpenGLVertexBuffer()
		{
		}


		OpenGLIndexBuffer::OpenGLIndexBuffer(const std::vector<unsigned int>& indices)
		{
		}

		OpenGLIndexBuffer::~OpenGLIndexBuffer()
		{
		}

	}
}
#endif