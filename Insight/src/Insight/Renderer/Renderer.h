#pragma once

#include "Insight/Core.h"

class MeshComponent;
class CameraComponent;
class Material;

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

	class IS_API Renderer
	{
	public:
		virtual ~Renderer() { }

		virtual void Clear() = 0;
		virtual void Render(CameraComponent* mainCamera, std::vector<MeshComponent*> meshes) = 0;
		virtual void Present() = 0;

		virtual Material* GetDefaultMaterial() { return nullptr; }

		static Renderer* Create();
	};
}


