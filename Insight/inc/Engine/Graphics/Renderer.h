#pragma once

#include "Engine/Core/Core.h"

class MeshComponent;
class CameraComponent;
class Material;
struct RendererData;

namespace Module
{
	class WindowModule;
}

enum class GraphicsRendererAPI
{
	None = 0,
	Vulkan = 1,
	OpenGL = 2
};

class IS_API Renderer
{
public:
	virtual ~Renderer() { }

	virtual void Init() = 0;
	virtual void Clear() = 0;
	virtual void Render(CameraComponent* mainCamera, std::vector<MeshComponent*>& meshes) = 0;
	virtual void Present() = 0;
	virtual void WaitForIdle() = 0;

	virtual Material* GetDefaultMaterial() { return nullptr; }

	GraphicsRendererAPI GetAPI() { return s_API; }

	static Renderer* New();

	static GraphicsRendererAPI s_API;
};