#pragma once

#include "Insight/Core/Core.h"
#include "Insight/Module/Module.h"
#include "Insight/Templates/TSingleton.h"
#include "Insight/Config/CVar.h"

#include "Insight/Renderer/Renderer.h"
#include "Insight/Renderer/ImGuiRenderer.h"

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
			GraphicsModule(SharedPtr<WindowModule> windowModule);
			virtual ~GraphicsModule() override;

			virtual void Update(const float& deltaTime) override;

			void WaitForIdle();
			GraphicsAPI GetAPI();
			void SetMainCamera(CameraComponent* camera);
			const bool HasMainCamera();
			const bool IsThisMainCamera(CameraComponent* camera);

			static Material* GetDefaultMaterial();

			struct GraphicsConfig
			{
				CVar<int> GraphicsAPI{ "graphics_api", 1 };
			};

		private:
			SharedPtr<WindowModule> m_windowModule;
			SharedPtr<Renderer> m_renderer;
			SharedPtr<ImGuiRenderer> m_imguiRenderer;

			static CameraComponent* m_mainCamera;
			static std::vector<WeakPtr<MeshComponent>> m_meshs;

			friend MeshComponent;
		};
	}
}

