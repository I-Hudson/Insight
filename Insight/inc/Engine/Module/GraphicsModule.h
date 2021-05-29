#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Module/Module.h"
#include "Engine/Templates/TSingleton.h"
#include "Engine/Module/WindowModule.h"

#include "Engine/Config/CVar.h"
#include "Engine/Graphics/ImGuiRenderer.h"

class CameraComponent;
namespace Insight::Module
{
	enum class GraphicsRendererAPI
	{
		None = 0,
		D311 = 1,
		D312 = 2,
		Vulkan = 3,
		OpenGL = 4
	};

	class GraphicsModule : public Module
								, public Core::TSingleton<GraphicsModule>
	{
	public:
		GraphicsModule();
		virtual ~GraphicsModule() override;

		virtual void OnCreate() override;
		virtual void Update(const float& deltaTime) override;

		void WaitForIdle();
		GraphicsRendererAPI GetAPI();

		bool IsD311();
		bool IsD312();
		bool IsVulkan();
		bool IsOpenGL();

#ifdef IS_EDITOR
		void SetEditorCamera(CameraComponent* camera) { m_editorCamera = camera; }
#endif

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
		void InitLoading();

		void ShadowMap();
		void Deffered();
		void Editor();

	private:
#ifdef IS_EDITOR
		CameraComponent* m_editorCamera;
#endif
		ImGuiRenderer* m_imguiRenderer;
		void* m_shadowMap = nullptr;
	};
}