#pragma once

#include "Runtime/Defines.h"

#include "ECS/Components/LightComponent.h"

#include "Generated/DirectionalLightComponent_reflect_generated.h"

namespace Insight
{
	namespace Graphics
	{
		class RHI_Texture;
	}

	namespace ECS
	{
		REFLECT_CLASS()
		class IS_RUNTIME DirectionalLightComponent : public LightComponent
		{
			REFLECT_GENERATED_BODY()
		public:
			IS_COMPONENT(DirectionalLightComponent);

			DirectionalLightComponent();
			virtual ~DirectionalLightComponent() override;

			IS_SERIALISABLE_H(DirectionalLightComponent);

			// Component - Begin
			/// @brief Called on creation (When the object is newed).
			virtual void OnCreate() override;
			/// @brief Called on destruction (Just before the object is deleted).
			virtual void OnDestroy() override;
			// Component - End

			LightComponentShadowMapResolutions GetShadowResolution() const;

			float GetCascadeSplitLambda() const;

			Graphics::RHI_Texture* GetShadowMap() const;

			const static u8 c_cascadeCount = 4;
		private:
			REFLECT_PROPERTY(EditorVisible);
			LightComponentShadowMapResolutions m_shadowMapResolution = LightComponentShadowMapResolutions::_1024;
			REFLECT_PROPERTY(EditorVisible, Meta(ClampMin(0.0), ClampMax(1.0)));
			float m_cascadeSplitLambda = 0.95f;

			Graphics::RHI_Texture* m_shadowCascadeMap = nullptr;
		};
	}

	OBJECT_SERIALISER(ECS::DirectionalLightComponent, 1,
		SERIALISE_BASE(ECS::LightComponent, 1, 0)
	);
}