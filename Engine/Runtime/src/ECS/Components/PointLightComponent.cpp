#include "ECS/Components/PointLightComponent.h"

#include "Graphics/RenderContext.h"

namespace Insight
{
	namespace ECS
	{
		PointLightComponent::PointLightComponent()
		{
		}

		PointLightComponent::~PointLightComponent()
		{ }

		IS_SERIALISABLE_CPP(PointLightComponent);

		void PointLightComponent::OnCreate()
		{
			ASSERT(!m_shadowCubemap);
			m_shadowCubemap = Renderer::CreateTexture();
			
			const u32 shadowMapResolution = static_cast<u32>(m_shadowMapResolution);

			Graphics::RHI_TextureInfo createInfo = Graphics::RHI_TextureInfo::Cubemap(
				shadowMapResolution, 
				shadowMapResolution, 
				PixelFormat::D32_Float, 
				Graphics::ImageUsageFlagsBits::DepthStencilAttachment | Graphics::ImageUsageFlagsBits::Sampled);
			m_shadowCubemap->Create(&Graphics::RenderContext::Instance(), createInfo);
		}

		void PointLightComponent::OnDestroy()
		{
			Renderer::FreeTexture(m_shadowCubemap);
		}

		float PointLightComponent::GetRadius() const
		{
			return m_radius;
		}

		LightComponentShadowMapResolutions PointLightComponent::GetShadowResolution() const
		{
			return m_shadowMapResolution;
		}

		Graphics::RHI_Texture* PointLightComponent::GetShadowMap() const
		{
			return m_shadowCubemap;
		}
	}
}