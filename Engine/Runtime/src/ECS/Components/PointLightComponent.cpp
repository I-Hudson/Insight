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
			
			u32 shadowMapResolution = 1024;
			//switch (m_shadowMapResolution)
			//{
			//case Insight::ECS::LightComponentShadowMapResolutions::_64:
			//	shadowMapResolution = 64;
			//	break;
			//case Insight::ECS::LightComponentShadowMapResolutions::_128:
			//	shadowMapResolution = 128;
			//	break;
			//case Insight::ECS::LightComponentShadowMapResolutions::_256:
			//	shadowMapResolution = 256;
			//	break;
			//case Insight::ECS::LightComponentShadowMapResolutions::_512:
			//	shadowMapResolution = 512;
			//	break;
			//case Insight::ECS::LightComponentShadowMapResolutions::_1024:
			//	shadowMapResolution = 1024;
			//	break;
			//case Insight::ECS::LightComponentShadowMapResolutions::_2048:
			//	shadowMapResolution = 2048;
			//	break;
			//default:
			//	break;
			//}
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

		Graphics::RHI_Texture* PointLightComponent::GetShadowMap() const
		{
			return m_shadowCubemap;
		}
	}
}