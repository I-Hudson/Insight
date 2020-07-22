#pragma once

#include "Insight/Core.h"

#include "glm/glm.hpp"

namespace Insight
{
	namespace Render
	{
		class Device;

		struct FrameBufferAttachment;

		class IS_API Framebuffer
		{
		public:
			Framebuffer(Device* device, const int& width, const int& height);
			Framebuffer(Device* device, const glm::vec2& extent);
			Framebuffer(Device* device, const uint32_t& format, const glm::vec2& extent);
			virtual ~Framebuffer() { }

			void CreateAttachment(const uint32_t& format, const uint32_t& usage, const uint32_t& imageLayout, const uint32_t& finalLayout);
			void CompileFrameBuffer();
			FrameBufferAttachment& GetAttachment(const int& index);

			void BindBuffer();
			void UnbindBuffer();

			void Resize(int width, int height);

		private:
			void CreateImage(FrameBufferAttachment& attachment);
			void CreateMemory(FrameBufferAttachment& attachment);
		};
	}
}
