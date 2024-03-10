#include "Editor/Premake/PremakeProjectTemplateData.h"

namespace Insight::Editor
{
    constexpr static const char* c_PremakeProjectTag_ProjectName = "--PROJECT_NAME";
    constexpr static const char* c_PremakeProjectTag_InsightPath = "--INSIGHT_PATH";
    constexpr static const char* c_PremakeProjectTag_Targetname = "--TARGET_NAME";
    constexpr static const char* c_PremakeProjectTag_PrebuildCommands = "--PREBUILD_COMMANDS";
    constexpr static const char* c_PremakeProjectTag_Defines = "--DEFINES";

    constexpr static const char* c_PremakeProjectFileName = "premake5_project.lua";

    constexpr static const char* c_PremakeProjectTemplate = R"(
    local insightPath = "--INSIGHT_PATH"
    includePath = "../../Content"
    local targetName="--TARGET_NAME"

    project "--PROJECT_NAME"
    kind "ConsoleApp"
    location "./"
    configurations { "Debug", "Release" } 

    targetname (targetName .. output_project_subfix)
    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
    debugdir ("bin/" .. outputdir .. "/%{prj.name}")

    dependson 
    {
    }

    defines
    {
        "IS_MONOLITH",
        "IS_STANDALONE",

        "_CRT_SECURE_NO_WARNINGS",
        "_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS",
        "_SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING",
        "_SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS",

        "GLM_FORCE_SWIZZLE",
        "GLM_FORCE_LEFT_HANDED",
        "GLM_FORCE_DEPTH_ZERO_TO_ONE",

        "IS_PLATFORM_X64",
        "IS_MEMORY_TRACKING",
        "RENDER_GRAPH_ENABLED",
        "TOBJECTPTR_REF_COUNTING",

        "COMMAND_LIST_RENDER_BATCH",
        "IS_NVIDIA_AFTERMATH_ENABLED",

        --DEFINES
    }

    includedirs
    {
        "../../Content/",
        "../../Intermediate/CodeFiles/",

        "%{IncludeDirs.InsightCore}",
        "%{IncludeDirs.InsightMaths}",
        "%{IncludeDirs.InsightInput}",
        "%{IncludeDirs.InsightGraphics}",
        "%{IncludeDirs.InsightRuntime}",
        insightPath .. "/Engine/Standalone/inc",

        "%{IncludeDirs.spdlog}",
        "%{IncludeDirs.doctest}",
        "%{IncludeDirs.imgui}",
        "%{IncludeDirs.implot}",
        "%{IncludeDirs.xxHash}",
        "%{IncludeDirs.glm}",
        "%{IncludeDirs.glfw}",
        "%{IncludeDirs.stb_image}",
        "%{IncludeDirs.splash}",
        "%{IncludeDirs.lz4}",
        "%{IncludeDirs.qoi}",
        "%{IncludeDirs.reflect}",
        "%{IncludeDirs.zip}",

        "%{IncludeDirs.assimp}",
        "%{IncludeDirs.assimp}/../build/include",
        "%{IncludeDirs.meshoptimizer}",
        "%{IncludeDirs.simplygon}",

        "%{IncludeDirs.vma}",
        "%{IncludeDirs.vulkan}",
        "%{IncludeDirs.spirv_reflect}",
        "%{IncludeDirs.FFR2}",
        "%{IncludeDirs.Aftermath}",
        "%{IncludeDirs.nvtx}",
        "%{IncludeDirs.DirectXHeaders}",
        "%{IncludeDirs.DirectXAgilityHeaders}",
        "%{IncludeDirs.DirectXShaderCompiler}",
        "%{IncludeDirs.D3D12MemoryAllocator}",

        "%{IncludeDirs.IconFontCppHeaders}",
        "%{IncludeDirs.nlohmann_json}",
    }

    files 
    { 
        insightPath .. "/Engine/Core/inc/**.hpp", 
        insightPath .. "/Engine/Core/inc/**.h",
        insightPath .. "/Engine/Core/inc/**.inl",
        insightPath .. "/Engine/Core/inc/**.cpp",
        insightPath .. "/Engine/Core/src/**.cpp",
        insightPath .. "/Engine/Core/src/**.inl",

        insightPath .. "/Engine/Maths/inc/**.hpp", 
        insightPath .. "/Engine/Maths/inc/**.h", 
        insightPath .. "/Engine/Maths/inc/**.inl",
        insightPath .. "/Engine/Maths/inc/**.cpp", 
        insightPath .. "/Engine/Maths/src/**.cpp", 
        insightPath .. "/Engine/Maths/src/**.inl",

        insightPath .. "/Engine/Input/inc/**.hpp", 
        insightPath .. "/Engine/Input/inc/**.h", 
        insightPath .. "/Engine/Input/inc/**.inl",
        insightPath .. "/Engine/Input/inc/**.cpp", 
        insightPath .. "/Engine/Input/src/**.cpp", 
        insightPath .. "/Engine/Input/src/**.inl",

        insightPath .. "/Engine/Graphics//inc/**.hpp", 
        insightPath .. "/Engine/Graphics/inc/**.h", 
        insightPath .. "/Engine/Graphics/inc/**.inl",
        insightPath .. "/Engine/Graphics/inc/**.cpp", 
        insightPath .. "/Engine/Graphics/src/**.cpp", 
        insightPath .. "/Engine/Graphics/src/**.inl",

        insightPath .. "/Engine/Runtime/inc/**.hpp", 
        insightPath .. "/Engine/Runtime/inc/**.h", 
        insightPath .. "/Engine/Runtime/inc/**.inl",
        insightPath .. "/Engine/Runtime/inc/**.cpp",
        insightPath .. "/Engine/Runtime/src/**.cpp",
        insightPath .. "/Engine/Runtime/src/**.inl",

        insightPath .. "/Engine/Standalone/inc/**.hpp", 
        insightPath .. "/Engine/Standalone/inc/**.h", 
        insightPath .. "/Engine/Standalone/inc/**.inl",
        insightPath .. "/Engine/Standalone/inc/**.cpp",
        insightPath .. "/Engine/Standalone/src/**.cpp",
        insightPath .. "/Engine/Standalone/src/**.inl",

        "../../Content/**.h", 
        "../../Content/**.c",
        "../../Content/**.hpp", 
        "../../Content/**.cpp",
        "../../Content/**.inl", 

        "../../Intermediate/CodeFiles/**.h", 
        "../../Intermediate/CodeFiles/**.c",
        "../../Intermediate/CodeFiles/**.hpp", 
        "../../Intermediate/CodeFiles/**.cpp",
        "../../Intermediate/CodeFiles/**.inl",

        --PROJECT_FILES

        insightPath .. "/vendor/stb/stb_image.h",
        insightPath .. "/vendor/stb/stb_image_write.h",
        insightPath .. "/vendor/SPIRV-Reflect/spirv_reflect.h",
        insightPath .. "/vendor/SPIRV-Reflect/spirv_reflect.cpp",
    }

    postbuildcommands
    {
        --PREBUILD_COMMANDS
    }    

    links
    {
        "GLFW.lib",
        "glm.lib",
        "imgui.lib",
        "zip.lib",

        "vulkan-1.lib",
        "D3d12.lib",
        "DXGI.lib",
        "dxguid.lib",
        "dxcompiler.lib",
        "meshoptimizer.lib",
        "GFSDK_Aftermath_Lib.x64.lib",
    }

    libdirs
    {
        "%{LibDirs.vulkan}",
        "%{LibDirs.deps_lib}",
    }

    filter "configurations:Debug or configurations:Testing"
        buildoptions "/MDd"
        defines
        {
            "_DEBUG",
            "IS_DEBUG",
        }
        links
        {
            "Splashd.lib",
            "glm.lib",
            "imgui.lib",
            "xxHashd.lib",
            "meshoptimizer.lib",
            "lz4d.lib",
            "Reflectd.lib",

            "GLFW.lib",
            "vulkan-1.lib",
            "D3d12.lib",
            "DXGI.lib",
            "dxguid.lib",
            "dxcompiler.lib",
            "GFSDK_Aftermath_Lib.x64.lib",

            "GenericCodeGend.lib",
            "glslangd.lib",
            "glslang-default-resource-limitsd.lib",
            "HLSLd.lib",
            "MachineIndependentd.lib",
            "OGLCompilerd.lib",
            "OSDependentd.lib",
            "SPIRVd.lib",
            "SPIRV-Toolsd.lib",
            "SPIRV-Tools-optd.lib",
            "SPVRemapperd.lib",
            "assimpd.lib",

            "ffx_fsr2_api_x64d.lib",
            "ffx_fsr2_api_vk_x64d.lib",
            "ffx_fsr2_api_dx12_x64d.lib",
        }

    filter "configurations:Release"
        buildoptions "/MD"
        optimize "On"   
                defines
        {
            "NDEBUG",
            "IS_RELEASE",
            "DOCTEST_CONFIG_DISABLE",
        }
        links
        {
            "Splash.lib",
            "glm.lib",
            "imgui.lib",
            "xxHash.lib",
            "meshoptimizer.lib",
            "lz4.lib",
            "Reflect.lib",

            "GLFW.lib",
            "vulkan-1.lib",
            "D3d12.lib",
            "DXGI.lib",
            "dxguid.lib",
            "dxcompiler.lib",
            "GFSDK_Aftermath_Lib.x64.lib",

            "glslang.lib",
            "MachineIndependent.lib",
            "GenericCodeGen.lib",
            "OGLCompiler.lib",
            "OSDependent.lib",
            "assimp.lib",

            "ffx_fsr2_api_x64.lib",
            "ffx_fsr2_api_vk_x64.lib",
            "ffx_fsr2_api_dx12_x64.lib",
        }

    filter "system:Windows"
    	system "windows"
    	toolset("msc-v143")
        defines
        {
            "IS_PLATFORM_WINDOWS",
            "IS_PLATFORM_WIN32",
            "IS_MATHS_DIRECTX_MATHS",
            "IS_DX12_ENABLED",
            "IS_VULKAN_ENABLED",
            "IS_CPP_WINRT",

            "NOMINMAX",

            "SPLASH_PLATFORM_WINDOWS",
            
            "VK_USE_PLATFORM_WIN32_KHR",
        }
        includedirs
         { 
            "%{IncludeDirs.pix}",
         }
        links
        {
            "WinPixEventRuntime.lib",
            "cppwinrt_fast_forwarder.lib",
            "Xinput.lib",
            "Comctl32.lib",
        }
        files
        {
            insightPath .. "/vendor/D3D12MemoryAllocator/src/D3D12MemAlloc.cpp",
            insightPath .. "/vendor/D3D12MemoryAllocator/include/D3D12MemAlloc.h",
        }

    filter { "system:Windows", "configurations:Debug" or "configurations:Testing" }
        ignoredefaultlibraries
        {
            "libcmt.lib",
            "msvcrt.lib",
            "libcmtd.lib",
        }
    filter { "system:Windows", "configurations:Release" }
        ignoredefaultlibraries
        {
            "libcmt.lib",
            "libcmtd.lib",
            "msvcrtd.lib",
        }

    filter "system:Unix"
    	system "linux"
    	toolset("clang")
        defines
        {
            "IS_PLATFORM_LINUX",
            "IS_VULKAN_ENABLED",
        }
    )";

    std::string CreatePackageBuildProjectFile(const PremakeProjectTemplateData& templateData);
}