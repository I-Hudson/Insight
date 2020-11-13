#pragma once

#include "Insight/Core.h"

class MeshComponent;
class CameraComponent;
class Material;
struct RendererData;

namespace Insight
{
	namespace Module
	{ class WindowModule; }

	enum class GraphicsAPI
	{
		None = 0,
		Vulkan = 1,
		OpenGL = 2
	};

	class IS_API Renderer : public Object
	{
	public:
		virtual ~Renderer() { }

		virtual void Clear() = 0;
		virtual void Render(CameraComponent* mainCamera, std::vector<MeshComponent*> meshes) = 0;
		virtual void Present() = 0;

		virtual Material* GetDefaultMaterial() { return nullptr; }

		static Renderer* Create();

		struct RendererConfig
		{
			CVar<int> VSync			{ "vsync", 0 };
			CVar<int> Validation	{ "validation", 1 };
		};

		static GraphicsAPI s_API;
	};
}


