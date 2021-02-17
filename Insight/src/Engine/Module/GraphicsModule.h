#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Module/Module.h"
#include "Engine/Templates/TSingleton.h"
#include "Engine/Module/WindowModule.h"

#include "Engine/Config/CVar.h"

#include "Engine/Graphics/Renderer.h"
#include "Engine/Graphics/ImGuiRenderer.h"

class MeshComponent;
class CameraComponent;
class Material;

namespace Module
{
	class IS_API GraphicsModule : public Module
								, public TSingleton<GraphicsModule>
	{
	public:
		GraphicsModule();
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
			CVar<int> Validation{ "validation", 1 };
			CVar<int> VSync{ "vsync", 1 };
			CVar<int> GSync{ "gsync", 1 };
			CVar<int> GraphicsAPI{ "graphics_api", 1 };

			CVar<int> MergeSubpasses{ "merge_subpasses", 1 };
			CVar<int> UseTransientDepthStencil{ "use_transient_depth_stencil", 1 };
			CVar<int> UseTransientColor{ "use_transient_color", 1 };

		};

	private:
		Renderer* m_renderer;
		ImGuiRenderer* m_imguiRenderer;

		static CameraComponent* m_mainCamera;
		static std::vector<MeshComponent*> m_meshs;

		friend MeshComponent;
	};
}