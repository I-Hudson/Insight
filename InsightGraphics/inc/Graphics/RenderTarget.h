#pragma once

#include "Graphics/PixelFormat.h"

#include <glm/glm.hpp>
#include<map>
#include <string>

namespace Insight
{
	namespace Graphics
	{
		class RenderTargetManager;
		class GPUImage;

		struct RenderTargetDesc
		{
			RenderTargetDesc()
			{ }
			RenderTargetDesc(int width, int height)
				: Width(width), Height(height)
			{ }
			RenderTargetDesc(int width, int height, PixelFormat format)
				: Width(width), Height(height), Format(format)
			{ }
			RenderTargetDesc(int width, int height, PixelFormat format, glm::vec4 clearColour)
				: Width(width), Height(height), Format(format), ClearColour(clearColour)
			{ }

			int Width;
			int Height;
			PixelFormat Format = PixelFormat::R8G8B8A8_UNorm;
			glm::vec4 ClearColour = glm::vec4(0, 0, 0, 1);
		};

		class RenderTarget
		{
		public:
			RenderTarget();
			~RenderTarget();

			RenderTargetDesc GetDesc() const { return m_desc; }
			GPUImage* GetImage() const { return m_image; }

		private:
			void Create(std::string key, RenderTargetDesc desc);
			void Destroy();

		private:
			RenderTargetDesc m_desc;
			GPUImage* m_image{ nullptr };

			friend class RenderTargetManager;
		};

		class RenderTargetManager
		{
		public:

			static RenderTargetManager& Instance()
			{
				static RenderTargetManager ins;
				return ins;
			}

			RenderTarget* GetOrCreateRenderTarget(std::string key, RenderTargetDesc desc);
			void Destroy();

		private:
			std::map<std::string, RenderTarget*> m_renderTargets;
		};
	}
}