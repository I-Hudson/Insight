local CommonConfig = require "../../lua/CommonConfig"

local EditorConfig = { }

function EditorConfig.DefinesSharedLib()
    defines
    {
        "IS_EDITOR_ENABLED",
        "IS_EXPORT_EDITOR_DLL",
    }
end

function EditorConfig.DefinesStaticLib()
    defines
    {
        "IS_EDITOR_ENABLED",
    }
end

function EditorConfig.IncludeDirs()
    includedirs
    {
        "inc",
        "%{IncludeDirs.InsightCore}",
        "%{IncludeDirs.InsightMaths}",
        "%{IncludeDirs.InsightPhysics}",
        "%{IncludeDirs.InsightInput}",
        "%{IncludeDirs.InsightGraphics}",
        "%{IncludeDirs.InsightRuntime}",
    
        "%{IncludeDirs.glfw}",
        "%{IncludeDirs.glm}",
        "%{IncludeDirs.spdlog}",
        "%{IncludeDirs.imgui}",
        "%{IncludeDirs.ImGuizmo}",
        "%{IncludeDirs.imgui_string}",
        "%{IncludeDirs.reflect}",
        "%{IncludeDirs.zip}",
        "%{IncludeDirs.splash}",
        "%{IncludeDirs.efsw}",
    }
end

function EditorConfig.LibraryDirs()
    libdirs
    {
        "%{wks.location}/vendor/glfw/lib",
    }
end

function EditorConfig.LibraryLinks(OutputProjectSubfix)
    links
    {
        "Insight_Core" .. OutputProjectSubfix .. ".lib",
        "Insight_Maths" .. OutputProjectSubfix .. ".lib",
        "Insight_Physics" .. OutputProjectSubfix .. ".lib",
        "Insight_Input" .. OutputProjectSubfix .. ".lib",
        "Insight_Graphics" .. OutputProjectSubfix .. ".lib",
        "Insight_Runtime" .. OutputProjectSubfix .. ".lib",

        "glm.lib",
        "imgui.lib",
        "zip.lib",
    }

    if (profileTool == "pix") then
        links
        {
            "WinPixEventRuntime.lib"
        }
    end
end

function EditorConfig.PostBuildCommands(OutputProjectSubfix, OutputDir)
    postbuildcommands
    {
        "{COPY} \"%{cfg.targetdir}/%{prj.name}" .. OutputProjectSubfix .. ".lib\" \"%{wks.location}deps/".. OutputDir..  "/lib/\"\n",
        "{COPY} \"%{cfg.targetdir}/%{prj.name}" .. OutputProjectSubfix .. ".pdb\" \"%{wks.location}deps/".. OutputDir..  "/pdb/\"\n",
    }
end

function EditorConfig.FilterConfigurations()
    CommonConfig.FilterConfigurations()

    filter "configurations:Debug or configurations:Testing"
        links
        {
            "Reflectd.lib",
            "Splashd.lib",
            "efswd.lib",
        }
        prebuildcommands 
        { 
            "{COPYDIR} \"%{wks.location}deps/" .. outputdir .. "/dll/\" \"%{cfg.targetdir}\"", 
            "{COPYDIR} \"%{wks.location}deps/" .. outputdir .. "/pdb/\" \"%{cfg.targetdir}\"",  
        }

    filter "configurations:Release"  
        links
        {
            "Reflect.lib",
            "Splash.lib",
            "efsw.lib",
        }
        prebuildcommands 
        { 
            "{COPYDIR} \"%{wks.location}deps/" .. outputdir .. "/dll/\" \"%{cfg.targetdir}\"", 
            "{COPYDIR} \"%{wks.location}deps/" .. outputdir .. "/pdb/\" \"%{cfg.targetdir}\"", 
        }

end

function EditorConfig.FilterPlatforms()
    filter "platforms:Win64"
        system "windows"
        files
        {
            "Insight_Editor.rc",
            "resource.h",
        }
end

return EditorConfig