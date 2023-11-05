#include "Editor/Premake/PremakeProjectTemplateData.h"

namespace Insight::Editor
{
    constexpr static const char* c_PremakeProjectTag_ProjectName = "--PROJECT_NAME";
    constexpr static const char* c_PremakeProjectTag_InsightPath = "--INSIGHT_PATH";
    constexpr static const char* c_PremakeProjectTag_Targetname = "--TARGET_NAME";
    constexpr static const char* c_PremakeProjectTag_AdditionalFiles = "--ADDITIONAL_FILES";
    constexpr static const char* c_PremakeProjectTag_VirtualPaths = "--VIRTUAL_PATHS";

    constexpr static const char* c_PremakeProjectFileName = "premake5_project.lua";

    constexpr static const char* c_PremakeProjectTemplate = R"(
    local insightPath = "--INSIGHT_PATH"
    includePath = "../../Content"
    local targetName="--TARGET_NAME"

    project "--PROJECT_NAME"
    kind "SharedLib"
    location "./"

    targetname (targetName .. output_project_subfix)
    targetdir ("../bin/" .. outputdir .. "/%{prj.name}")
    objdir ("../bin-int/" .. outputdir .. "/%{prj.name}")
    debugdir ("../bin/" .. outputdir .. "/%{prj.name}")

    dependson 
    {
    }

    defines
    {
        "IS_EDITOR_ENABLED",
        "IS_EXPORT_PROJECT_DLL",
        "IS_EXPORT_DLL",
    }

    includedirs
    {
        "%{includePath}",
        "%{IncludeDirs.InsightCore}",
        "%{IncludeDirs.InsightMaths}",
        "%{IncludeDirs.InsightInput}",
        "%{IncludeDirs.InsightGraphics}",
        "%{IncludeDirs.InsightRuntime}",
        "%{IncludeDirs.InsightEditor}",

        "%{IncludeDirs.glfw}",
        "%{IncludeDirs.glm}",
        "%{IncludeDirs.spdlog}",
        "%{IncludeDirs.imgui}",
        "%{IncludeDirs.imgui_string}",
        "%{IncludeDirs.reflect}",
    }

    files 
    { 
        "%{includePath}" .. "/**.hpp", 
        "%{includePath}" .. "/**.h", 
        "%{includePath}" .. "/**.inl", 
        "%{includePath}" .. "/**.cpp",
        "%{includePath}" .. "/**.c",

        --ADDITIONAL_FILES
    }

    vpaths
    {
        --VIRTUAL_PATHS
    }

    links
    {
        "Insight_Core" .. output_project_subfix .. ".lib",
        "Insight_Maths" .. output_project_subfix .. ".lib",
        "Insight_Input" .. output_project_subfix .. ".lib",
        "Insight_Graphics" .. output_project_subfix .. ".lib",
        "Insight_Runtime" .. output_project_subfix .. ".lib",
        "Insight_Editor" .. output_project_subfix .. ".lib",

        "imgui.lib",
        "glm.lib",
    }

    if (profileTool == "pix") then
        links
        {
            "WinPixEventRuntime.lib"
        }
    end

    libdirs
    {
        insightPath .. "/deps/lib",
    }

    filter "configurations:Debug or configurations:Testing"
        defines { "DEBUG" }  
        symbols "On" 
        links
        {
            "xxHashd.lib",
            "lz4d.lib",
            "Reflectd.lib",
        }
        libdirs
        {
            insightPath .. "/deps/lib/debug",
        }

    filter "configurations:Release"  
        defines { "NDEBUG" }    
        optimize "On" 
        links
        {
            "xxHash.lib",
            "lz4.lib",
            "Reflect.lib",
        }
        libdirs
        {
            "%{wks.location}/deps/lib/release",
        }

    filter "platforms:Win64"
        links
        {
            "Ole32.lib",
            "dbghelp.lib",
            "Rpcrt4.lib",
        }
    )";

    std::string CreatePremakeProjectTemplateFile(const PremakeProjectTemplateData& templateData);
}