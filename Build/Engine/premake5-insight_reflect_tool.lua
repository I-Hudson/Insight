local profileTool="tracy"

local InsightEngineIncludes = require "lua/InsightEngineIncludes"
local InsightVendorIncludes = require "lua/InsightVendorIncludes"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
outputdir_target = "%{wks.location}bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
outputdir_obj = "%{wks.location}bin-int/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
outputdir_debug = "%{wks.location}bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
output_project_subfix = ""
output_executable = ""
output_executable = "Insight_Standalone"


VULKAN_SDK = os.getenv("VULKAN_SDK")

IncludeDirs = {}
IncludeDirs["insight_editor"] = "%{wks.location}Engine/Editor/inc"
IncludeDirs["reflect"] = "%{wks.location}vendor/Reflect/Reflect/inc"
IncludeDirs["tracy"] = "%{wks.location}vendor/tracy"
IncludeDirs["pix"] = "%{wks.location}vendor/winpixeventruntime/Include"
InsightEngineIncludes.AddIncludes(IncludeDirs, "%{wks.location}")
InsightVendorIncludes.AddIncludes(IncludeDirs, "%{wks.location}")


LibDirs = {}
LibDirs["deps_lib"] = "%{wks.location}deps/" .. outputdir .. "/lib/"
LibDirs["deps_testing_lib"] = "%{wks.location}deps/Debug-windows-x86_64/lib/"

workspace "InsightReflectTool"
    architecture "x64"
    startproject "InsightReflectTool"
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
        --"Linux",
    }

    defines
    {
        "REFLECT_DLL_IMPORT",
    }

    flags
    {
    	"MultiProcessorCompile"
    }

    includedirs
    {

    }

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

    if (profileTool == "pix") then
        links
        {
            "WinPixEventRuntime.lib"
        }
    end

include "../../Engine/ReflectTool/premake.lua"

newaction{
    trigger = "clean",
    description = "Remove all binaries and intermediate binaries, and vs files.",
    execute = function()
        print("Removing symlinks")
        os.execute("Remove_Symlinks.bat")
        print("Removeing binaries")
        os.rmdir("../../bin")
        print("Removeing internediate binaries")
        os.rmdir("../../bin-int")
        print("Removing dependencies")
        os.rmdir("../../deps")
        print("Removeing project files")
        os.rmdir("../../.vs")
        os.remove("../../Engine/**.sln")
        os.remove("../../Engine/**.vcxproj")
        os.remove("../../Engine/**.vcxproj.filters")
        os.remove("../../Engine/**.vcxproj.user")
        print("Done")
    end
} 