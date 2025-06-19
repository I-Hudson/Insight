#include "ECS/Components/DirectionalLightComponent.h"

#include "Graphics/RenderContext.h"

namespace Insight
{
	namespace ECS
	{
		DirectionalLightComponent::DirectionalLightComponent()
		{
		}

		DirectionalLightComponent::~DirectionalLightComponent()
		{ }

		IS_SERIALISABLE_CPP(DirectionalLightComponent);

		void DirectionalLightComponent::OnCreate()
		{
			ASSERT(!m_shadowCascadeMap);
			m_shadowCascadeMap = Renderer::CreateTexture();
			
			const u32 shadowMapResolution = static_cast<u32>(m_shadowMapResolution);

			Graphics::RHI_TextureInfo createInfo = Graphics::RHI_TextureInfo::Tex2DArray(
				shadowMapResolution, 
				shadowMapResolution, 
				PixelFormat::D32_Float, 
				Graphics::ImageUsageFlagsBits::DepthStencilAttachment | Graphics::ImageUsageFlagsBits::Sampled,
				c_cascadeCount);
			m_shadowCascadeMap->Create(&Graphics::RenderContext::Instance(), createInfo);
		}

		void DirectionalLightComponent::OnDestroy()
		{
			Renderer::FreeTexture(m_shadowCascadeMap);
		}

		LightComponentShadowMapResolutions DirectionalLightComponent::GetShadowResolution() const
		{
			return m_shadowMapResolution;
		}

		float DirectionalLightComponent::GetCascadeSplitLambda() const
		{
			return m_cascadeSplitLambda;
		}

		Graphics::RHI_Texture* DirectionalLightComponent::GetShadowMap() const
		{
			return m_shadowCascadeMap;
		}
	}
}