local CommonConfig = require "../lua/CommonConfig"
local RuntimeConfig = require "lua/RuntimeConfig"

project "Insight_Runtime"  
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
        "Insight_Physics",
        "Insight_Graphics",
        "Insight_Input",
    }

    files 
    { 
        "inc/**.hpp", 
        "inc/**.h", 
        "inc/**.inl", 
        "src/**.cpp",
        "src/**.c",
        "src/**.inl",

        "inc/**.cpp", 

        "**.natvis",

        "../Core/inc/Memory/NewDeleteOverload.h", 
        "../Core/src/Memory/NewDeleteOverload.cpp",

        "../../vendor/stb/stb_image.h",
        "../../vendor/stb/stb_image_write.h",
    }

    RuntimeConfig.DefinesSharedLib()

    RuntimeConfig.IncludeDirs()
    RuntimeConfig.LibraryDirs()
    RuntimeConfig.LibraryLinks()

    RuntimeConfig.FilterConfigurations()
    RuntimeConfig.FilterPlatforms()

    CommonConfig.PostBuildCopyLibraryToOutput()