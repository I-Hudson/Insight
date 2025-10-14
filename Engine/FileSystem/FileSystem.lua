local CommonConfig = require "../lua/CommonConfig"
local FileSystemConfig = require "lua/FileSystemConfig"

project "Insight_FileSystem"  
    configurations { "Debug", "Release" } 
    location "./"
    
    targetname ("%{prj.name}" .. output_project_subfix)
    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
    debugdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")

    dependson 
    {
        "Insight_Core",
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

    FileSystemConfig.DefinesSharedLib()

    FileSystemConfig.IncludeDirs()
    FileSystemConfig.LibraryDirs()
    FileSystemConfig.LibraryLinks()

    FileSystemConfig.FilterConfigurations()
    FileSystemConfig.FilterPlatforms()

    CommonConfig.PostBuildCopyLibraryToOutput()
