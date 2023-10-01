local InsightEngineIncludes = require "lua/InsightEngineIncludes"
local InsightVendorIncludes = require "lua/InsightVendorIncludes"

local profileTool="tracy"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
outputdir_target = "%{wks.location}bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
outputdir_obj = "%{wks.location}bin-int/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
outputdir_debug = "%{wks.location}bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
output_project_subfix = ""
output_executable = ""


VULKAN_SDK = os.getenv("VULKAN_SDK")

IncludeDirs = {}
InsightEngineIncludes.AddIncludes(IncludeDirs)
InsightVendorIncludes.AddIncludes(IncludeDirs)

LibDirs = {}
LibDirs["deps_lib"] = "%{wks.location}deps/" .. outputdir .. "/lib/"
LibDirs["deps_testing_lib"] = "%{wks.location}deps/Debug-windows-x86_64/lib/"

workspace "InsightTools"
    architecture "x64"
    staticruntime "on"
    location "../../"

    language ("C++")
    cppdialect ("C++17")

    configurations
    {
        "Debug",
        "Release",
    }
    platforms 
    { 
        "Win64", 
        "Linux",
    }

    flags
    {
    	"MultiProcessorCompile"
    }

    defines
    {
        "_CRT_SECURE_NO_WARNINGS",
        "_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS",
        
        "REFLECT_DLL_EXPORT",
        "REFLECT_TYPE_INFO_ENABLED",
        "IS_PLATFORM_X64",
    }

    includedirs
    {
        "%{IncludeDirs.InsightCore}",
        "%{IncludeDirs.InsightMaths}",
        "%{IncludeDirs.InsightGraphics}",
        "%{IncludeDirs.InsightInput}",
        "%{IncludeDirs.InsightRuntime}",

        "%{IncludeDirs.tracy}",
        "%{IncludeDirs.doctest}",
    }
    InsightVendorIncludes.AddIncludesToIncludeDirs(includedirs)

    if (profileTool == "tracy") then
        defines { "IS_PROFILE_ENABLED", "IS_PROFILE_TRACY", "TRACY_IMPORTS", "TRACY_ON_DEMAND", "TRACY_CALLSTACK", }
        includedirs { "%{IncludeDirs.tracy}", }
    end
    if (profileTool == "pix") then
        defines { "IS_PROFILE_ENABLED", "IS_PROFILE_PIX", "USE_PIX" }
        includedirs { "%{IncludeDirs.pix}", }
    end 

    libdirs
    {
        "%{LibDirs.deps_lib}",
    }

    links
    {
        "Insight_Core.lib",
        "Insight_Maths.lib",
        "Insight_Graphics.lib",
        "Insight_Input.lib",
        "Insight_Runtime.lib",

        "tracy.lib",

    }

    if (profileTool == "pix") then
        links
        {
            "WinPixEventRuntime.lib"
        }
    end

    filter "configurations:Debug or configurations:Testing"
        buildoptions "/MDd"
        defines
        {
            "_DEBUG",
            "IS_DEBUG",
        }
        links
        {
            "Reflectd.lib",
        }
        prebuildcommands { "{COPYDIR} \"%{wks.location}deps/" .. outputdir .. "/dll/Reflectd.dll\" \"%{cfg.targetdir}\"" }
        prebuildcommands { "{COPYDIR} \"%{wks.location}deps/" .. outputdir .. "/dll/\" \"%{cfg.targetdir}\"", "{COPYDIR} \"%{wks.location}deps/" .. outputdir .. "/pdb/\" \"%{cfg.targetdir}\"",  }

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
        }
        prebuildcommands { "{COPYDIR} \"%{wks.location}deps/" .. outputdir .. "/dll/Reflect.dll\" \"%{cfg.targetdir}\"" }
        prebuildcommands { "{COPYDIR} \"%{wks.location}deps/" .. outputdir .. "/dll/\" \"%{cfg.targetdir}\"", "{COPYDIR} \"%{wks.location}deps/" .. outputdir .. "/pdb/\" \"%{cfg.targetdir}\"", }


        filter "system:Windows"
    	system "windows"
    	toolset("msc-v143")
        defines
        {
            "IS_PLATFORM_WINDOWS",
            "IS_PLATFORM_WIN32",
            --"IS_MATHS_DIRECTX_MATHS",
            "IS_DX12_ENABLED",
            "IS_VULKAN_ENABLED",
            "IS_CPP_WINRT",
            
            "USE_PIX",
            "NOMINMAX",

            "SPLASH_PLATFORM_WINDOWS",
            
            "VK_USE_PLATFORM_WIN32_KHR",
        }
        links
        {
            "WinPixEventRuntime.lib",
            "cppwinrt_fast_forwarder.lib",
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


include "../../Engine/Tools/AssetPacker/premake.lua"