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
    }

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
    	toolset("msc")
        defines
        {
            "IS_PLATFORM_WINDOWS",
            "IS_PLATFORM_WIN32",
        }
    	
    filter "system:Unix"
    	system "linux"
    	toolset("clang")
        defines
        {
            "IS_PLATFORM_LINUX",
        }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

VULKAN_SDK = os.getenv("VULKAN_SDK")

IncludeDirs = {}
IncludeDirs["InsightCore"] = "%{wks.location}/InsightCore/inc"
IncludeDirs["InsightGraphics"] = "%{wks.location}/InsightGraphics/inc"
IncludeDirs["InsightApp"] = "%{wks.location}/InsightApp/inc"
IncludeDirs["InsightEditor"] = "%{wks.location}/InsightEditor/inc"

IncludeDirs["glfw"] = "%{wks.location}/Vendor/glfw/include"
IncludeDirs["glm"] = "%{wks.location}/Vendor/glm"
IncludeDirs["vulkan"] = VULKAN_SDK .. "/include/"

LibDirs = {}
LibDirs["vulkan"] = VULKAN_SDK .. "/lib/"

group "Dependices"
        include "vendor/glm/glm.lua"
group "Runtime"
        include "InsightCore/InsightCore.lua"
        include "InsightGraphics/InsightGraphics.lua"
        include "InsightApp/InsightApp.lua"
group "Editor"
    include "InsightEditor/InsightEditor.lua"
