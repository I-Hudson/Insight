local CommonConfig = require "../lua/CommonConfig"
local InputConfig = require "lua/InputConfig"

project "Insight_Input"  
    configurations { "Debug", "Release" } 
    location "./"
    
    targetname ("%{prj.name}" .. output_project_subfix)
    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
    debugdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")

    dependson 
    {
        "Insight_Core",
        "Insight_Maths",
    }

    files 
    { 
        "inc/**.hpp", 
        "inc/**.h", 
        "inc/**.inl", 
        "src/**.cpp",
        "src/**.inl",

        "../Core/inc/Memory/NewDeleteOverload.h", 
        "../Core/src/Memory/NewDeleteOverload.cpp",
    }

    InputConfig.DefinesSharedLib()

    InputConfig.IncludeDirs()
    InputConfig.LibraryDirs()
    InputConfig.LibraryLinks()

    InputConfig.FilterConfigurations()
    InputConfig.FilterPlatforms()

    CommonConfig.PostBuildCopyLibraryToOutput()
