local CommonConfig = require "../lua/CommonConfig"
local MathsConfig = require "lua/MathsConfig"

project "Insight_Maths"  
    location "./"

    targetname ("%{prj.name}" .. output_project_subfix)
    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
    debugdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")

    dependson 
    {
    }

    files 
    { 
        "inc/**.hpp", 
        "inc/**.h", 
        "inc/**.inl", 
        "src/**.cpp",
        "src/**.inl",
    }

    MathsConfig.DefinesSharedLib()

    MathsConfig.IncludeDirs()
    MathsConfig.LibraryDirs()
    MathsConfig.LibraryLinks()

    MathsConfig.FilterConfigurations()
    MathsConfig.FilterPlatforms()

    CommonConfig.PostBuildCopyLibraryToOutput()