#pragma once

#include "Insight/Core.h"

class MeshComponent;
namespace Insight
{
	namespace Module
	{ class WindowModule; }
	class Camera;

	struct RendererStartUpData
	{
		Module::WindowModule* WindowModule;
	};

	enum class GraphicsAPI
	{
		None = 0,
		Vulkan = 1,
		OpenGL = 2
	};

	class IS_API Renderer
	{
	public:
		virtual ~Renderer() { }

		virtual void Clear() = 0;
		virtual void Render(Camera* mainCamera, std::vector<MeshComponent*> meshes) = 0;
		virtual void Present() = 0;

		static Renderer* Create(RendererStartUpData& startupData);
	};
}


