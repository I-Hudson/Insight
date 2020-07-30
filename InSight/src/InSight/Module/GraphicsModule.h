#pragma once

#include "Insight/Core.h"
#include "Insight/Module/Module.h"
#include "Insight/Config/CVar.h"

#include "Insight/Renderer/Renderer.h"

class MeshComponent;

namespace Insight
{
	namespace Module
	{
		class WindowModule;

		class IS_API GraphicsModule : public Module
		{
		public:
			GraphicsModule(WindowModule* windowModule);
			virtual ~GraphicsModule() override;

			virtual void Update(const float& deltaTime) override;

			static void SetMainCamera(Camera* camera);


			struct GraphicsConfig
			{
				CVar<int> GraphicsAPI{ "graphics_api", 1 };
			};

		private:
			WindowModule* m_windowModule;
			Renderer* m_renderer;

			static Camera* m_mainCamera;
			static std::vector<MeshComponent*> m_meshs;

			friend MeshComponent;
		};
	}
}

