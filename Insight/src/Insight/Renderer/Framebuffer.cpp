#include "ispch.h"
#include "Insight/Renderer/Framebuffer.h"

namespace Insight
{
	namespace Render
	{
		Framebuffer::Framebuffer(Device* device, const int& width, const int& height)
		{
		}
		Framebuffer::Framebuffer(Device* device, const glm::vec2& extent)
		{
		}
		Framebuffer::Framebuffer(Device* device, const uint32_t& format, const glm::vec2& extent)
		{
		}
		void Framebuffer::CreateAttachment(const uint32_t& format, const uint32_t& usage, const uint32_t& imageLayout, const uint32_t& finalLayout)
		{
		}
		void Framebuffer::CompileFrameBuffer()
		{
		}
		FrameBufferAttachment& Framebuffer::GetAttachment(const int& index)
		{
			// TODO: insert return statement here
			FrameBufferAttachment a;
			return a;
		}
		void Framebuffer::BindBuffer()
		{
		}
		void Framebuffer::UnbindBuffer()
		{
		}
		void Framebuffer::Resize(int width, int height)
		{
		}
		void Framebuffer::CreateImage(FrameBufferAttachment& attachment)
		{
		}
		void Framebuffer::CreateMemory(FrameBufferAttachment& attachment)
		{
		}
	}
}