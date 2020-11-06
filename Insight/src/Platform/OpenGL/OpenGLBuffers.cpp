#include "ispch.h"

#include "Platform/OpenGL/OpenGL.h"
#include "Platform/OpenGL/OpenGLBuffers.h"

namespace Platform
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