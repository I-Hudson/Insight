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
        "%{IncludeDirs.nvrhi}",
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
        "glm",
        "imgui",
        "zip",
    }

    if (profileTool == "pix") then
        links
        {
            "WinPixEventRuntime"
        }
    end
end

function EditorConfig.PostBuildCommands(OutputProjectSubfix, OutputDir)
    postbuildcommands
    {
        "{COPYFILE} \"%{cfg.targetdir}/%{prj.name}" .. OutputProjectSubfix .. ".lib\" \"%{wks.location}deps/".. OutputDir..  "/lib/\"\n",
        "{COPYFILE} \"%{cfg.targetdir}/%{prj.name}" .. OutputProjectSubfix .. ".pdb\" \"%{wks.location}deps/".. OutputDir..  "/pdb/\"\n",
    }
    prebuildcommands 
    { 
        "{COPYDIR} \"%{wks.location}deps/" .. outputdir .. "/dll/\" \"%{cfg.targetdir}\"", 
    }
end

function EditorConfig.FilterConfigurations()
    CommonConfig.FilterConfigurations()

    filter "configurations:Debug or configurations:Testing"
        links
        {
            "Reflectd",
            "Splashd",
            "efswd",
        }

    filter "configurations:Release"  
        links
        {
            "Reflect",
            "Splash",
            "efsw",
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