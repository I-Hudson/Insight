#pragma once

#include "Graphics/PixelFormat.h"

#include "Maths/Vector4.h"

#include <string>
#include <map>

namespace Insight
{
	namespace Graphics
	{
		class RenderTargetManager;
		class RHI_Texture;

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
			RenderTargetDesc(int width, int height, PixelFormat format, Maths::Vector4 clearColour)
				: Width(width), Height(height), Format(format), ClearColour(clearColour)
			{ }

			int Width;
			int Height;
			PixelFormat Format = PixelFormat::R8G8B8A8_UNorm;
			Maths::Vector4 ClearColour = Maths::Vector4(0, 0, 0, 1);
		};

		class RenderTarget
		{
		public:
			RenderTarget();
			~RenderTarget();

			void Create(std::string key, RenderTargetDesc desc);
			void Destroy();

			RenderTargetDesc GetDesc() const { return m_desc; }
			RHI_Texture* GetTexture() const { return m_texture; }

		private:

		private:
			RenderTargetDesc m_desc;
			std::string m_key;
			RHI_Texture* m_texture{ nullptr };

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