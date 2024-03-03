#include "Editor/Editor.h"

#include "EditorModule.h"
#include "Editor/EditorWindows/ProjectWindow.h"
#include "Editor/EditorWindows/GameViewWindow.h"
#include "Editor/EditorWindows/WorldViewWindow.h"

#include "Asset/AssetRegistry.h"
#include "Asset/AssetPackage/IAssetPackage.h"

#include "Runtime/EntryPoint.h"
#include "Resource/ResourceManager.h"

#include "Runtime/RuntimeEvents.h"
#include "Runtime/ProjectSystem.h"

#include "Core/Memory.h"
#include "Core/ImGuiSystem.h"
#include "Core/EnginePaths.h"

#include "Graphics/Window.h"
#include "Graphics/RenderContext.h"

#include "Serialisation/Archive.h"

#include <SplashScreen.h>

namespace Insight
{
    namespace Editor
    {
        IS_SERIALISABLE_CPP(Editor);

        SplashScreen splashScreen;

        Editor::Editor()
        {
        }

        Editor::~Editor()
        {
        }

        void Editor::OnPreInit()
        {
            static const std::string splashScreenBackGroundPath = EnginePaths::GetResourcePath() + "/Insight/cover.png";
            splashScreen.Init(860, 420);
            splashScreen.SetBackgroundImage(splashScreenBackGroundPath.c_str());
            splashScreen.Show();
        }

        void Editor::OnInit()
        {
            IS_PROFILE_FUNCTION();

            const std::string engineResources = EnginePaths::GetInstallDirectory() + "/EngineResources.zip";
            if (FileSystem::Exists(engineResources))
            {
                Runtime::AssetRegistry::Instance().LoadAssetPackage(engineResources);
            }
            else
            {
                Runtime::IAssetPackage* editorIconsAssetPackage = Runtime::AssetRegistry::Instance().CreateAssetPackage("EngineResources");
                Runtime::AssetRegistry::Instance().AddAssetsInFolder(EnginePaths::GetResourcePath(), editorIconsAssetPackage, true, false);
            }

            const std::string editorAssetPackagePath = EnginePaths::GetResourcePath() + "/Editor/EditorIcons.isassetpackage";
            if (FileSystem::Exists(editorAssetPackagePath))
            {
                Runtime::IAssetPackage* editorIconsAssetPackage =
                    Runtime::AssetRegistry::Instance().LoadAssetPackage(editorAssetPackagePath);
            }
            else
            {
                const std::string editorIconsPath = EnginePaths::GetResourcePath() + "/Editor";
                Runtime::IAssetPackage* editorIconsAssetPackage = Runtime::AssetRegistry::Instance().CreateAssetPackage("EditorIcons");
                Runtime::AssetRegistry::Instance().AddAssetsInFolder(editorIconsPath, editorIconsAssetPackage, true, false);
            }

            m_projectAssetPackage = Runtime::AssetRegistry::Instance().CreateAssetPackage(c_ProjectAssetPackageName);
            m_contentListener.SetAssetPackage(m_projectAssetPackage);

            Core::EventSystem::Instance().AddEventListener(this, Core::EventType::Project_Open, [this](const Core::Event& e)
                {
                    const Runtime::ProjectInfo& projectInfo = Runtime::ProjectSystem::Instance().GetProjectInfo();

                    Runtime::AssetRegistry::Instance().AddAssetsInFolder(projectInfo.GetContentPath(), m_projectAssetPackage, true, false);
                    Runtime::ResourceManager::Instance().LoadResourcesInFolder(projectInfo.GetContentPath(), true);

                    Runtime::AssetRegistry::Instance().SetDebugDirectories(projectInfo.GetIntermediatePath() + "/AssetMeta", projectInfo.GetContentPath());
                    Runtime::ResourceManager::Instance().SetDebugDirectories(projectInfo.GetIntermediatePath() + "/Meta", projectInfo.GetContentPath());

                    std::vector<const Runtime::AssetInfo*> allAssetPackages = Runtime::AssetRegistry::Instance().GetAllAssetsWithExtension(Runtime::IAssetPackage::c_FileExtension);
                    for (const Runtime::AssetInfo* info : allAssetPackages)
                    {
                        if (!Runtime::AssetRegistry::Instance().LoadAssetPackage(info->GetFullFilePath()))
                        {
                            FAIL_ASSERT();
                        }
                    }

                    m_contentListener.WatchId = m_fileWatcher.addWatch(projectInfo.GetContentPath(), &m_contentListener, true);
                    m_fileWatcher.watch();
                });

            m_editorResourceManager.Initialise();

            EditorModule::Initialise(GetSystemRegistry().GetSystem<Core::ImGuiSystem>());

            std::string windowTitle = "Insight Editor";
#ifdef IS_DEBUG
            windowTitle += " Debug ";
#elif IS_RELEASE
            windowTitle += " Release ";
#endif
            windowTitle += "(";
            windowTitle += Graphics::GraphicsAPIToString(Graphics::RenderContext::Instance().GetGraphicsAPI());
            windowTitle += ")";

            Graphics::Window::Instance().SetTite(windowTitle);
            Graphics::Window::Instance().SetIcon(EnginePaths::GetResourcePath() + "/Insight/default.png");
            Graphics::Window::Instance().Show();

            m_buildSystem.Initialise();
            m_hotReloadSystem.Initialise();
            App::Engine::Instance().GetSystemRegistry().RegisterSystem(&m_buildSystem);
            App::Engine::Instance().GetSystemRegistry().RegisterSystem(&m_hotReloadSystem);

            m_editorWindowManager.RegisterWindows();
            m_editorWindowManager.AddWindow(WorldViewWindow::WINDOW_NAME);

            m_menuBar.Initialise(&m_editorWindowManager);

            Archive editorSettings(c_EditorSettingsFileName, ArchiveModes::Read);
            if (!editorSettings.IsEmpty())
            {
                EditorSettingsSerialiser serialiser(true);
                serialiser.Deserialise(editorSettings.GetData());
                Deserialise(&serialiser);
            }

            for (auto& world : Runtime::WorldSystem::Instance().GetAllWorlds())
            {
                world->SetWorldState(Runtime::WorldStates::Paused);
            }
        }

        void Editor::OnPostInit()
        {
            m_gameRenderpass = New<Graphics::Renderpass>();
            m_gameRenderpass->Create();

            if (Runtime::ProjectSystem::Instance().IsProjectOpen())
            {
                const Runtime::ProjectInfo& projectInfo = Runtime::ProjectSystem::Instance().GetProjectInfo();

                Runtime::IResource* resource = Runtime::ResourceManager::Instance().LoadSync(
                    Runtime::ResourceId(Runtime::ProjectSystem::Instance().GetProjectInfo().GetContentPath() + "/Textures/Background.png"
                        , Runtime::Texture2D::GetStaticResourceTypeId())).Get();
            }

            if (!Runtime::ProjectSystem::Instance().IsProjectOpen())
            {
                m_editorWindowManager.AddWindow(ProjectWindow::WINDOW_NAME);
                const ProjectWindow* projectWindow = static_cast<const ProjectWindow*>(m_editorWindowManager.GetActiveWindow(ProjectWindow::WINDOW_NAME));
                RemoveConst(projectWindow)->SetFullscreen(true);
            }
            else
            {
                m_editorWindowManager.RemoveWindow(ProjectWindow::WINDOW_NAME);
            }

            splashScreen.Destroy();
        }

        void Editor::OnUpdate()
        {
            IS_PROFILE_FUNCTION();
            
            m_menuBar.Draw();
            EditorWindowManager::Instance().Update();
            m_contentListener.Update();
        }

        void Editor::OnRender()
        {
            IS_PROFILE_FUNCTION();

            const bool gameViewWindowActive = m_editorWindowManager.GetActiveWindow(GameViewWindow::WINDOW_NAME) != nullptr;
            m_gameRenderpass->FrameSetup();
            m_gameRenderpass->RenderMainPasses(gameViewWindowActive);
            m_gameRenderpass->RenderSwapchain(false);
            m_gameRenderpass->RenderPostprocessing();
        }

        void Editor::OnDestroy()
        {
            IS_PROFILE_FUNCTION();

            Core::EventSystem::Instance().RemoveEventListener(this, Core::EventType::Project_Open);
            m_fileWatcher.removeWatch(m_contentListener.WatchId);

            EditorSettingsSerialiser serialiser(false);
            Serialise(&serialiser);

            Archive editorSettings(c_EditorSettingsFileName, ArchiveModes::Write);
            editorSettings.Write(serialiser.GetSerialisedData());
            editorSettings.Close();

            m_hotReloadSystem.Shutdown();
            m_buildSystem.Shutdown();

            m_gameRenderpass->Destroy();
            Delete(m_gameRenderpass);

            EditorWindowManager::Instance().Destroy();

            App::Engine::Instance().GetSystemRegistry().UnregisterSystem(&m_hotReloadSystem);
            App::Engine::Instance().GetSystemRegistry().UnregisterSystem(&m_buildSystem);

            m_editorResourceManager.Shutdown();

            Runtime::AssetRegistry::Instance().Shutdown();
        }
    }
}

Insight::App::Engine* CreateApplication()
{
    return New<Insight::Editor::Editor, Insight::Core::MemoryAllocCategory::Editor>();
}
