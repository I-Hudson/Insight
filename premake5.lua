local profileTool="tracy"

workspace "Insight"
    architecture "x64"
    startproject "InsightEditor"
    staticruntime "off"

    configurations
    {
        "Debug",
        "Release",
        "Testing",
    }
    platforms 
    { 
        "Win64", 
        "Linux" 
    }

    flags
    {
    	"MultiProcessorCompile"
    }

    defines
    {
        "_CRT_SECURE_NO_WARNINGS",
        "GLM_FORCE_SWIZZLE",
        "IS_PLATFORM_X64",
        "RENDER_GRAPH_ENABLED",
    }

    includedirs
    {
        "%{IncludeDirs.optick}",
        "%{IncludeDirs.tracy}",
        "%{IncludeDirs.doctest}",
    }

    if (profileTool == "tracy") then
        defines { "IS_PROFILE_ENABLED", "IS_PROFILE_TRACY", "TRACY_IMPORTS", "TRACY_ON_DEMAND", }
        editandcontinue "off"
    end
    if (profileTool == "optick") then
        defines { "IS_PROFILE_ENABLED", "IS_PROFILE_OPTICK" }
    end

    libdirs
    {
        "%{LibDirs.deps_lib}",
    }

    filter "configurations:Debug"
        defines
        {
            "DOCTEST_CONFIG_DISABLE",
        }

    filter { "configurations:Debug", "configurations:Testing" }
        buildoptions "/MDd"
        defines
        {
            "_DEBUG"
        }

    filter "configurations:Release"
        buildoptions "/MD"
        defines
        {
            "NDEBUG",
            "DOCTEST_CONFIG_DISABLE",
        }

    filter "system:Windows"
    	system "windows"
    	toolset("msc-v142")
        defines
        {
            "IS_PLATFORM_WINDOWS",
            "IS_PLATFORM_WIN32",
            "VK_USE_PLATFORM_WIN32_KHR",
            "IS_DX12_ENABLED",
            "IS_VULKAN_ENABLED",
            "NOMINMAX",
        }


    filter "configurations:Testing"
        defines
        {
            "TESTING",
            "DOCTEST_CONFIG_IMPLEMENTATION_IN_DLL",
        }
        files 
        { 
            "vendor/doctest/doctest/doctest.h",
        } 
    	
    
    filter "system:Unix"
    	system "linux"
    	toolset("clang")
        defines
        {
            "IS_PLATFORM_LINUX",
            "IS_VULKAN_ENABLED",
        }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
outputdir_target = "/bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
outputdir_obj = "/bin-int/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
outputdir_debug = "bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
output_executable = "InsightEditor"

VULKAN_SDK = os.getenv("VULKAN_SDK")

IncludeDirs = {}
IncludeDirs["InsightCore"] = "%{wks.location}/InsightCore/inc"
IncludeDirs["InsightGraphics"] = "%{wks.location}/InsightGraphics/inc"
IncludeDirs["InsightECS"] = "%{wks.location}/InsightECS/inc"
IncludeDirs["InsightApp"] = "%{wks.location}/InsightApp/inc"
IncludeDirs["InsightEditor"] = "%{wks.location}/InsightEditor/inc"

IncludeDirs["doctest"] = "%{wks.location}/vendor/doctest/doctest"
IncludeDirs["glfw"] = "%{wks.location}/Vendor/glfw/include"
IncludeDirs["glm"] = "%{wks.location}/Vendor/glm"
IncludeDirs["imgui"] = "%{wks.location}/Vendor/imgui"
IncludeDirs["spdlog"] = "%{wks.location}/Vendor/spdlog/include"
IncludeDirs["vma"] = "%{wks.location}/vendor/VulkanMemoryAllocator/src"
IncludeDirs["vulkan"] = VULKAN_SDK .. "/include/"
IncludeDirs["spirv_reflect"] = "%{wks.location}/vendor/SPIRV-Reflect"
IncludeDirs["dxcompiler"] = "%{wks.location}/vendor/dxcompiler/win_debug/inc"
IncludeDirs["assimp"] = "%{wks.location}/vendor/assimp/include"
IncludeDirs["optick"] = "%{wks.location}/vendor/optick/src"
IncludeDirs["tracy"] = "%{wks.location}/vendor/tracy"
IncludeDirs["stb_image"] = "%{wks.location}/vendor/stb"

LibDirs = {}

LibDirs["deps_lib"] = "%{wks.location}/deps/" .. outputdir .. "/lib/"
LibDirs["deps_testing_lib"] = "%{wks.location}/deps/Debug-windows-x86_64/lib/"

LibDirs["glslang_win_d"] = "%{wks.location}/vendor/glslang/win_debug/lib"
LibDirs["glslang_win"] = "%{wks.location}/vendor/glslang/win_release/lib"
LibDirs["imgui"] = "%{wks.location}/vendor/imgui/" .. outputdir .. "ImGui/"
LibDirs["dxcompiler_win_d"] = "%{wks.location}/vendor/dxcompiler/win_debug/lib/x64"
LibDirs["dxcompiler_win"] = "%{wks.location}/vendor/dxcompiler/win_release/lib/x64"
LibDirs["vulkan"] = VULKAN_SDK .. "/lib/"

group "Runtime"
        include "InsightCore/InsightCore.lua"
        include "InsightGraphics/InsightGraphics.lua"
        include "InsightECS/InsightECS.lua"
        include "InsightApp/InsightApp.lua"
group "Editor"
    include "InsightEditor/InsightEditor.lua"
