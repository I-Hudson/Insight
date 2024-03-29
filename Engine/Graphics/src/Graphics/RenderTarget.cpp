#include "Graphics/RenderTarget.h"
#include "Graphics/RenderContext.h"


#include "Graphics/PixelFormatExtensions.h"

#include "Event/EventSystem.h"
#include "Core/Profiler.h"

namespace Insight
{
	namespace Graphics
	{
		RenderTarget::RenderTarget()
		{
			Core::EventSystem::Instance().AddEventListener(this, Core::EventType::Graphics_Swapchain_Resize, [this](const Core::Event& e)
				{
					IS_PROFILE_SCOPE("RenderTarget-Graphics_Swapchain_Resize");
					const Core::GraphcisSwapchainResize& event = static_cast<const Core::GraphcisSwapchainResize&>(e);
					m_desc.Width = event.Width;
					m_desc.Height = event.Height;
					Create(m_key, m_desc);
				});
		}

		RenderTarget::~RenderTarget()
		{
			Core::EventSystem::Instance().RemoveEventListener(this, Core::EventType::Graphics_Swapchain_Resize);
			Destroy();
		}

		void RenderTarget::Create(std::string key, RenderTargetDesc desc)
		{
			Destroy();
			m_key = key;
			m_desc = desc;

			m_texture = Renderer::CreateTexture();

			Graphics::RHI_TextureInfo textureInfo = {};
			textureInfo.TextureType = TextureType::Tex2D;
			textureInfo.Width = m_desc.Width;
			textureInfo.Height = m_desc.Height;
			textureInfo.Format = m_desc.Format;

			if (PixelFormatExtensions::IsDepth(textureInfo.Format))
			{
				textureInfo.ImageUsage = 
					ImageUsageFlagsBits::DepthStencilAttachment
					| ImageUsageFlagsBits::Sampled;
			}
			else
			{
				textureInfo.ImageUsage = 
					ImageUsageFlagsBits::ColourAttachment
					| ImageUsageFlagsBits::Sampled;
			}
			m_texture->Create(&RenderContext::Instance(), textureInfo);


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