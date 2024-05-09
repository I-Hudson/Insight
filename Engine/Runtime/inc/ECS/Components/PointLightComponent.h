#pragma once

#include "Runtime/Defines.h"

#include "ECS/Components/LightComponent.h"

#include "Generated/PointLightComponent_reflect_generated.h"

namespace Insight
{
	namespace Graphics
	{
		class RHI_Texture;
	}

	namespace ECS
	{
		REFLECT_CLASS()
		class IS_RUNTIME PointLightComponent : public LightComponent
		{
			REFLECT_GENERATED_BODY()
		public:
			IS_COMPONENT(PointLightComponent);

			PointLightComponent();
			virtual ~PointLightComponent() override;

			IS_SERIALISABLE_H(PointLightComponent);

			// Component - Begin
			/// @brief Called on creation (When the object is newed).
			virtual void OnCreate() override;
			/// @brief Called on destruction (Just before the object is deleted).
			virtual void OnDestroy() override;
			// Component - End

			float GetRadius() const;

			Graphics::RHI_Texture* GetShadowMap() const;

		private:
			REFLECT_PROPERTY(EditorVisible);
			float m_radius = 16.0f;
			//LightComponentShadowMapResolutions m_shadowMapResolution = LightComponentShadowMapResolutions::_1024;

			Graphics::RHI_Texture* m_shadowCubemap = nullptr;
		};
	}

	OBJECT_SERIALISER(ECS::PointLightComponent, 1,
		SERIALISE_PROPERTY(float, m_radius, 1, 0)
		SERIALISE_BASE(ECS::LightComponent, 1, 0)
	);
}