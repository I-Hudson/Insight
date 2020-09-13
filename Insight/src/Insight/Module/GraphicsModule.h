#pragma once

#include "Insight/Core.h"
#include "Insight/Module/Module.h"
#include "Insight/Templates/TSingleton.h"
#include "Insight/Config/CVar.h"

#include "Insight/Renderer/Renderer.h"

class MeshComponent;
class CameraComponent;
class Material;

namespace Insight
{
	namespace Module
	{
		class WindowModule;

		class IS_API GraphicsModule : public Module
			, public TSingleton<GraphicsModule>
		{
		public:
			GraphicsModule(WindowModule* windowModule);
			virtual ~GraphicsModule() override;

			virtual void Update(const float& deltaTime) override;

			void SetMainCamera(CameraComponent* camera);
			const bool HasMainCamera();
			const bool IsThisMainCamera(CameraComponent* camera);

			static Material* GetDefaultMaterial();

			struct GraphicsConfig
			{
				CVar<int> GraphicsAPI{ "graphics_api", 1 };
			};

		private:
			WindowModule* m_windowModule;
			Renderer* m_renderer;

			static CameraComponent* m_mainCamera;
			static std::vector<MeshComponent*> m_meshs;

			friend MeshComponent;
		};
	}
}

