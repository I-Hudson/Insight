#include "Editor/Editor.h"

#include "EditorModule.h"
#include "Editor/EditorWindows/ProjectWindow.h"
#include "Editor/EditorWindows/GameViewWindow.h"
#include "Editor/EditorWindows/WorldViewWindow.h"

#include "Runtime/EntryPoint.h"
#include "Resource/ResourceManager.h"
#include "Resource/ResourcePack.h"

#include "Runtime/RuntimeEvents.h"
#include "Runtime/ProjectSystem.h"

#include "Core/Memory.h"
#include "Core/ImGuiSystem.h"
#include "Core/EnginePaths.h"

#include "Graphics/Window.h"
#include "Graphics/RenderContext.h"

#include "Serialisation/Archive.h"

#include <imgui.h>
#include <SplashScreen.h>

namespace Insight
{
    namespace Editor
    {
        IS_SERIALISABLE_CPP(Editor);

        SplashScreen splashScreen;

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

            Core::EventSystem::Instance().AddEventListener(this, Core::EventType::Project_Open, [](const Core::Event& e)
                {
                    const Runtime::ProjectInfo& projectInfo = Runtime::ProjectSystem::Instance().GetProjectInfo();
                    Runtime::AssetRegistry::Instance().AddAssetsInFolder(projectInfo.GetContentPath());
                });
            Runtime::AssetRegistry::Instance().AddAssetsInFolder(EnginePaths::GetResourcePath(), true);


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

            m_editorWindowManager.RegisterWindows();
            m_editorWindowManager.AddWindow(WorldViewWindow::WINDOW_NAME);

            m_menuBar.Initialise(&m_editorWindowManager);

            m_gameRenderpass = New<Graphics::Renderpass>();
            m_gameRenderpass->Create();

            App::Engine::Instance().GetSystemRegistry().RegisterSystem(&m_hotReloadSystem);
            App::Engine::Instance().GetSystemRegistry().RegisterSystem(&m_buildSystem);

            m_buildSystem.Initialise();
            m_hotReloadSystem.Initialise();

            Archive editorSettings(c_EditorSettingsFileName, ArchiveModes::Read);
            if (!editorSettings.IsEmpty())
            {
                EditorSettingsSerialiser serialiser(true);
                serialiser.Deserialise(editorSettings.GetData());
                Deserialise(&serialiser);
            }
        }

        void Editor::OnPostInit()
        {
            const Runtime::ProjectInfo& projectInfo = Runtime::ProjectSystem::Instance().GetProjectInfo();
            const Runtime::AssetInfo* assetInfo = Runtime::AssetRegistry::Instance().AddAsset(projectInfo.GetContentPath() + "/Txt.txt");
            assetInfo = Runtime::AssetRegistry::Instance().AddAsset(projectInfo.GetContentPath() + "/Textures/Christmas_Cute_Roadhog.png");

            Runtime::ResourcePack* pack = Runtime::ResourceManager::CreateResourcePack(
                Runtime::ProjectSystem::Instance().GetProjectInfo().GetContentPath() + "/Pack");

            Runtime::IResource* resource = Runtime::ResourceManager::LoadSync(
                Runtime::ResourceId(Runtime::ProjectSystem::Instance().GetProjectInfo().GetContentPath() + "/Textures/Background.png"
                    , Runtime::Texture2D::GetStaticResourceTypeId())).Get();

            pack->AddResource(resource);

            splashScreen.Destroy();
        }

        void Editor::OnUpdate()
        {
            IS_PROFILE_FUNCTION();

            if (!Runtime::ProjectSystem::Instance().IsProjectOpen())
            {
                m_editorWindowManager.AddWindow(ProjectWindow::WINDOW_NAME);
                const ProjectWindow* projectWindow = static_cast<const ProjectWindow*>(m_editorWindowManager.GetActiveWindow(ProjectWindow::WINDOW_NAME));
                RemoveConst(projectWindow)->SetFullscreen(true);
            }
            else
            {
                m_menuBar.Draw();
            }

            EditorWindowManager::Instance().Update();
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

            EditorSettingsSerialiser serialiser(false);
            Serialise(&serialiser);

            Archive editorSettings(c_EditorSettingsFileName, ArchiveModes::Write);
            editorSettings.Write(serialiser.GetSerialisedData());
            editorSettings.Close();

            m_gameRenderpass->Destroy();
            Delete(m_gameRenderpass);

            EditorWindowManager::Instance().Destroy();

            m_hotReloadSystem.Shutdown();
            m_buildSystem.Shutdown();

            App::Engine::Instance().GetSystemRegistry().UnregisterSystem(&m_hotReloadSystem);
            App::Engine::Instance().GetSystemRegistry().UnregisterSystem(&m_buildSystem);

            Runtime::ResourceManager::SaveDatabase();
        }
    }
}

Insight::App::Engine* CreateApplication()
{
    return New<Insight::Editor::Editor, Insight::Core::MemoryAllocCategory::Editor>();
}
