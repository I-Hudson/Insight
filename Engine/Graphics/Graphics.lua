local CommonConfig = require "../lua/CommonConfig"
local GraphicsConfig = require "lua/GraphicsConfig"

project "Insight_Graphics"  
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
        "Insight_Input",
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

        "../../vendor/SPIRV-Reflect/spirv_reflect.h",
        "../../vendor/SPIRV-Reflect/spirv_reflect.cpp",
        "../../vendor/stb/stb_image.h",
        "../../vendor/stb/stb_image_write.h",
    }

    GraphicsConfig.DefinesSharedLib()
    GraphicsConfig.IncludeDirs()
    GraphicsConfig.LibraryDirs()
    GraphicsConfig.LibraryLinks()
   
    GraphicsConfig.FilterConfigurations()
    GraphicsConfig.FilterPlatforms()
    
    CommonConfig.PostBuildCopyLibraryToOutput(output_project_subfix, outputdir, output_executable)