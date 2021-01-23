#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Module/Module.h"
#include "Engine/Templates/TSingleton.h"
#include "Engine/Config/CVar.h"

#include "Engine/Graphics/Renderer.h"
#include "Engine/Graphics/ImGuiRenderer.h"

class MeshComponent;
class CameraComponent;
class Material;

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