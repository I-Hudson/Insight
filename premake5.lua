workspace "Insight"
    architecture "x64"
    startproject "InsightEditor"
    staticruntime "off"

    configurations
    {
        "Debug",
        "Release",
    }

    flags
    {
    	"MultiProcessorCompile"
    }

    defines
    {
        "IS_PROFILE_OPTICK",
        "_CRT_SECURE_NO_WARNINGS",
        "GLM_FORCE_SWIZZLE",
        "PLATFORM_X64",
    }

    includedirs
    {
        "%{IncludeDirs.optick}",
    }

    editandcontinue "Off"

    filter "configurations:Debug"
        defines
        {
            "_DEBUG"
        }
        buildoptions "/MDd"

    filter "configurations:Release"
        defines
        {
            "NDEBUG"
        }
        buildoptions "/MD"

        filter "system:Windows"
    	system "windows"
    	toolset("msc-v143")
        defines
        {
            "IS_PLATFORM_WINDOWS",
            "IS_PLATFORM_WIN32",
            "VK_USE_PLATFORM_WIN32_KHR",
        }
    	
    filter "system:Unix"
    	system "linux"
    	toolset("clang")
        defines
        {
            "IS_PLATFORM_LINUX",
        }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
outputdir_target = "/bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
outputdir_obj = "/bin-int/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
outputdir_debug = "bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

VULKAN_SDK = os.getenv("VULKAN_SDK")

IncludeDirs = {}
IncludeDirs["InsightCore"] = "%{wks.location}/InsightCore/inc"
IncludeDirs["InsightGraphics"] = "%{wks.location}/InsightGraphics/inc"
IncludeDirs["InsightApp"] = "%{wks.location}/InsightApp/inc"
IncludeDirs["InsightEditor"] = "%{wks.location}/InsightEditor/inc"

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

LibDirs = {}
LibDirs["glslang_win_d"] = "%{wks.location}/vendor/glslang/win_debug/lib"
LibDirs["glslang_win"] = "%{wks.location}/vendor/glslang/win_release/lib"
LibDirs["imgui"] = "%{wks.location}/vendor/imgui/" .. outputdir .. "ImGui/"
LibDirs["dxcompiler_win_d"] = "%{wks.location}/vendor/dxcompiler/win_debug/lib/x64"
LibDirs["dxcompiler_win"] = "%{wks.location}/vendor/dxcompiler/win_release/lib/x64"
LibDirs["vulkan"] = VULKAN_SDK .. "/lib/"

group "Dependices"
        include "vendor/glm/glm.lua"
        include "vendor/imgui/imgui.lua"
group "Runtime"
        include "InsightCore/InsightCore.lua"
        include "InsightGraphics/InsightGraphics.lua"
        include "InsightApp/InsightApp.lua"
group "Editor"
    include "InsightEditor/InsightEditor.lua"
