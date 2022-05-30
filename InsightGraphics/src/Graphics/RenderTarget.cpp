#include "Graphics/RenderTarget.h"
#include "Graphics/RenderContext.h"
#include "Graphics/GraphicsManager.h"

#include "Graphics/PixelFormatExtensions.h"

namespace Insight
{
	namespace Graphics
	{
		RenderTarget::RenderTarget()
		{
		}

		RenderTarget::~RenderTarget()
		{
			Destroy();
		}

		void RenderTarget::Create(std::string key, RenderTargetDesc desc)
		{
			Destroy();
			m_desc = desc;

			m_texture = Renderer::CreateTexture();

			Graphics::RHI_TextureCreateInfo textureInfo = {};
			textureInfo.TextureType = TextureType::Tex2D;
			textureInfo.Width = m_desc.Width;
			textureInfo.Height = m_desc.Height;
			textureInfo.Format = m_desc.Format;

			if (PixelFormatExtensions::IsDepth(textureInfo.Format))
			{
				textureInfo.ImageUsage = ImageUsageFlagsBits::DepthStencilAttachment;
			}
			else
			{
				textureInfo.ImageUsage = ImageUsageFlagsBits::ColourAttachment;
			}
			m_texture->Create(GraphicsManager::Instance().GetRenderContext(), textureInfo);


		}

		void RenderTarget::Destroy()
		{
			if (m_texture)
			{
				Renderer::FreeTexture(m_texture);
				m_texture = nullptr;
			}
		}
	}
}