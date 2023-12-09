#include "Editor/Premake/PremakeSolutionTemplateData.h"

namespace Insight::Editor
{
    constexpr static const char* c_PremakeSolutionTag_SolutionName = "--SOLUTION_NAME";
    constexpr static const char* c_PremakeSolutionTag_ProjectName = "--PROJECT_NAME";
    constexpr static const char* c_PremakeSolutionTag_InsightPath = "--INSIGHT_PATH";
    constexpr static const char* c_PremakeSolutionTag_SolutionLocation = "--SOLUTION_LOCATION";

    constexpr static const char* c_PremakeSolutionFileName = "premake5_solution.lua";

    constexpr static const char* c_PremakeSolutionTemplate = R"(
VULKAN_SDK = os.getenv("VULKAN_SDK")
local insightPath = "--INSIGHT_PATH"

local InsightEngineIncludes = require(insightPath .. "/Build/Engine/lua/InsightEngineIncludes")
local InsightVendorIncludes = require(insightPath .. "/Build/Engine/lua/InsightVendorIncludes")

local profileTool="tracy"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
output_project_subfix = ""

IncludeDirs = {}
InsightEngineIncludes.AddIncludes(IncludeDirs, insightPath .. "/")
InsightVendorIncludes.AddIncludes(IncludeDirs, insightPath .. "/")

LibDirs = {}
LibDirs["deps_lib"] = insightPath .. "/deps/" .. outputdir .. "/lib/"
LibDirs["deps_testing_lib"] = insightPath .. "/deps/Debug-windows-x86_64/lib/"

LibDirs["imgui"] = insightPath .. "/vendor/imgui/" .. outputdir .. "ImGui/"
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
        "_SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING",
        "_SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS",

        "GLM_FORCE_SWIZZLE",
        "GLM_FORCE_LEFT_HANDED",
        "GLM_FORCE_DEPTH_ZERO_TO_ONE",

        "REFLECT_TYPE_INFO_ENABLED",
        "REFLECT_DLL_IMPORT",

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
     
    postbuildcommands
    { 
        "{COPYDIR} \"" .. insightPath .. "/deps/" .. outputdir .. "/dll/\" \"%{cfg.targetdir}\"", 
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

    include "premake5_project.lua"
    )";

    std::string CreatePackageBuildSolutionFile(const PremakeSolutionTemplateData& templateData);
}