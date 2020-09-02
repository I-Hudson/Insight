#pragma once

#include "Insight/Core.h"

#include "glm/glm.hpp"

namespace Platform
{
	struct FrameBufferAttachment;
}


namespace Insight
{
	namespace Render
	{
		class Device;

		class IS_API Framebuffer
		{
		public:
			virtual ~Framebuffer() { }

			virtual void CreateAttachment(const uint32_t& format, const uint32_t& usage, const uint32_t& imageLayout, const uint32_t& finalLayout) = 0;
			virtual void CompileFrameBuffer() = 0;
			virtual Platform::FrameBufferAttachment& GetAttachment(const int& index) = 0;

			virtual void Resize(int width, int height) = 0;
		};
	}
}
