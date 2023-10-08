#include "Editor/Premake/PremakeSolutionTemplateData.h"

namespace Insight::Editor
{
    constexpr static const char* c_PremakeSolutionTag_SolutionName = "--SOLUTION_NAME";
    constexpr static const char* c_PremakeSolutionTag_ProjectName = "--PROJECT_NAME";
    constexpr static const char* c_PremakeSolutionTag_InsightPath = "--INSIGHT_PATH";
    constexpr static const char* c_PremakeSolutionTag_SolutionLocation = "--SOLUTION_LOCATION";
    constexpr static const char* c_PremakeSolutionTag_PremakeProjectFileName = "--PREMAKE_PROJECT_FILE_NAME";

    constexpr static const char* c_PremakeSolutionFileName = "premake5_solution.lua";

    constexpr static const char* c_PremakeSolutionTemplate = R"(
local InsightEngineIncludes = require "../../../Build/Engine/lua/InsightEngineIncludes"
local InsightVendorIncludes = require "../../../Build/Engine/lua/InsightVendorIncludes"

local profileTool="tracy"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
outputdir_target = "%{wks.location}bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
outputdir_obj = "%{wks.location}bin-int/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
outputdir_debug = "%{wks.location}bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
output_project_subfix = ""
output_executable = ""
output_executable = "Insight_Standalone"


VULKAN_SDK = os.getenv("VULKAN_SDK")

IncludeDirs = {}
InsightEngineIncludes.AddIncludes(IncludeDirs)
InsightVendorIncludes.AddIncludes(IncludeDirs)

LibDirs = {}
LibDirs["deps_lib"] = "%{wks.location}deps/" .. outputdir .. "/lib/"
LibDirs["deps_testing_lib"] = "%{wks.location}deps/Debug-windows-x86_64/lib/"

LibDirs["imgui"] = "%{wks.location}vendor/imgui/" .. outputdir .. "ImGui/"
LibDirs["vulkan"] = VULKAN_SDK .. "/lib/"

workspace "--SOLUTION_NAME"
    architecture "x64"
    startproject "--PROJECT_NAME"
    staticruntime "on"
    location "--SOLUTION_LOCATION"

    language ("C++")
    cppdialect ("C++17")

    configurations
    {
        "Debug",
        "Release"
    }
    platforms 
    { 
        "Win64"
    }

    defines
    {
        "_CRT_SECURE_NO_WARNINGS",
        "_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS",

        "GLM_FORCE_SWIZZLE",
        "GLM_FORCE_LEFT_HANDED",
        "GLM_FORCE_DEPTH_ZERO_TO_ONE",

        "REFLECT_TYPE_INFO_ENABLED",

        "IS_PLATFORM_X64",
        "IS_MEMORY_TRACKING",
        "RENDER_GRAPH_ENABLED",
        "TOBJECTPTR_REF_COUNTING",
    }

    flags
    {
    	"MultiProcessorCompile"
    }

    includedirs
    {
        "%{IncludeDirs.tracy}",
    }
     
    prebuildcommands
     { 
        "{COPYDIR} \"%{wks.location}deps/" .. outputdir .. "/dll/\" \"%{cfg.targetdir}\"", 
    }


    if (profileTool == "tracy") then
        editandcontinue "off"
        defines { "IS_PROFILE_ENABLED", "IS_PROFILE_TRACY", "TRACY_IMPORTS", "TRACY_ON_DEMAND", }
        filter "configurations:Debug or configurations:Testing"
            links { "tracy.lib", }
        filter { "configurations:Release" }
            links { "tracy.lib", }
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

    include "--PREMAKE_PROJECT_FILE_NAME"
} 
    )";

    std::string CreatePremakeSolutionTemplateFile(const char* outFolder, const PremakeSolutionTemplateData& templateData);
}