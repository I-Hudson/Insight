#include "Editor/HotReload/HotReloadLibrary.h"
#include "Editor/HotReload/HotReloadExportFunctions.h"
#include "Editor/HotReload/HotReloadEvents.h"

#include "Core/Logger.h"
#include "Core/ImGuiSystem.h"

#include "Runtime/Engine.h"

#include "FileSystem/FileSystem.h"
#include "Platforms/Platform.h"

namespace Insight::Editor
{
    HotReloadLibrary::HotReloadLibrary()
    {
    }

    HotReloadLibrary::~HotReloadLibrary()
    {
    }

    void HotReloadLibrary::Load(std::string_view libraryPath)
    {
        if (IsLoaded())
        {
            IS_CORE_ERROR("[HotReloadLibrary::Load] Unable to load library at '{0}' as a library is already loaded. Please 'Unload' before trying to load a new library.", libraryPath);
            return;
        }

        if (!FileSystem::Exists(libraryPath))
        {
            IS_CORE_ERROR("[HotReloadLibrary::Load] Unable to load library at '{0}' as the path is invalid.", libraryPath);
            return;
        }

        m_libraryHandle = Platform::LoadDynamicLibrary(libraryPath);
        if (IsLoaded())
        {
            auto initialiseFunc = Platform::GetDynamicFunction<void, Core::ImGuiSystem*>(m_libraryHandle, ProjectModule::c_Initialise);
            Core::ImGuiSystem* imguiSystem = App::Engine::Instance().GetSystemRegistry().GetSystem<Core::ImGuiSystem>();
            ASSERT(initialiseFunc != nullptr);
            ASSERT(imguiSystem != nullptr);
            initialiseFunc(imguiSystem);
        }
        else
        {
            IS_CORE_ERROR("[HotReloadLibrary::Load] Unable to load library at '{0}'. Unknown why.", libraryPath);
        }

        Core::EventSystem::Instance().DispatchEventNow(MakeRPtr<HotReloadLibraryLoaded>());
    }

    void HotReloadLibrary::Unload()
    {
        if (!IsLoaded())
        {
            IS_CORE_INFO("[HotReloadLibrary::Unload] Library is already unloaded.");
            return;
        }

        auto uninitialiseFunc = Platform::GetDynamicFunction<void>(m_libraryHandle, ProjectModule::c_Uninitialise);
        ASSERT(uninitialiseFunc != nullptr);
        uninitialiseFunc();

        Platform::FreeDynamicLibrary(m_libraryHandle);
        Core::EventSystem::Instance().DispatchEventNow(MakeRPtr<HotReloadLibraryUnLoaded>());
    }

    bool HotReloadLibrary::IsLoaded() const
    {
        return m_libraryHandle != nullptr;
    }

    HotReloadMetaData HotReloadLibrary::GetMetaData() const
    {
        if (!IsLoaded())
        {
            IS_CORE_INFO("[HotReloadLibrary::GetMetaData] Library is not loaded.");
            return {};
        }
        auto getMetaData = Platform::GetDynamicFunction<HotReloadMetaData>(m_libraryHandle, ProjectModule::c_GetMetaData);
        ASSERT(getMetaData != nullptr);
        return getMetaData();
    }

    std::string_view HotReloadLibrary::GetName() const
    {
        return m_name;
    }

    std::string_view HotReloadLibrary::GetPath() const
    {
        return m_path;
    }

    std::string_view HotReloadLibrary::GetFullPath() const
    {
        return m_fullPath;
    }

    void* HotReloadLibrary::GetLibraryHandle() const
    {
        return m_libraryHandle;
    }
}