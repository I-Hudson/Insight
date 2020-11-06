#pragma once

#include "Insight/Core.h"

#include "Insight/Renderer/Buffer.h"

#include <GLFW/glfw3.h>

namespace Platform
{
	class OpenGLRenderer;

	class IS_API OpenGLVertexBuffer : public Insight::Render::VertexBuffer
	{
	public:
		OpenGLVertexBuffer(const std::vector<Vertex>& vertices);
		virtual ~OpenGLVertexBuffer() override;

		unsigned int GetBuffer() const { return m_buffer; }

	private:
		unsigned int m_buffer;

		static OpenGLRenderer* s_Renderer;
		friend OpenGLRenderer;
	};

	class OpenGLIndexBuffer : public Insight::Render::IndexBuffer
	{
	public:
		OpenGLIndexBuffer(const std::vector<unsigned int>& indices);
		virtual ~OpenGLIndexBuffer() override;

		unsigned int  GetBuffer() const { return m_buffer; }

	private:
		unsigned int m_buffer;

		static OpenGLRenderer* s_Renderer;
		friend OpenGLRenderer;
	};
}