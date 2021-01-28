#pragma once

#include "Core.h"
#include <memory>
#include <thread>
#include <mutex>

class Scene;
namespace Module
{
	class ModuleManager;
	class WindowModule;
	class GraphicsModule;
	class InputModule;
	class AssetModule;
}

namespace FileSystem
{
	class FileSystemManager;
}

enum UpdateThreadState
{
	SAME_FRMAE = 0,
	ONE_FRAME_AHEAD,
	TWO_FRAME_AHEAD
};

class IS_API Application
{
public:
	Application();
	virtual ~Application();

	virtual void Create() = 0;
	virtual void Update(const float deltaTime) = 0;
	virtual void Draw() = 0;
	virtual void OnFrameEnd() { }

	void Run();
	void RenderLoop();

	static uint64_t FrameCount;

protected:
	std::mutex m_mutex;
	std::thread m_updateThread;
	std::thread m_renderThread;

	UpdateThreadState m_updateThreadState;

	bool m_isRunning = true;
	bool m_triggerRender = true;
	bool m_renderComplete = true;
	Scene* m_utitledScene;

	FileSystem::FileSystemManager* m_fileSystem;

	Module::ModuleManager* m_moduleManager;
	Module::WindowModule* m_windowModule;
	Module::GraphicsModule* m_graphicsModule;
	Module::InputModule* m_inputModule;
};

Application* CreateApplication();