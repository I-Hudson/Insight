local local_post_build_commands = post_build_commands

local CommonConfig = require "../lua/CommonConfig"
local PhysicsConfig = require "lua/PhysicsConfig"

project "Insight_Physics"  
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
        "src/**.c",
        "src/**.inl",
        "inc/**.cpp", 

        "**.natvis",
        "../../vendor/JoltPhysics/Jolt/Jolt.natvis",
    }

    PhysicsConfig.DefinesSharedLib()
    PhysicsConfig.IncludeDirs()

    PhysicsConfig.LibraryLinks()
    PhysicsConfig.LibraryDirs()

    CommonConfig.PostBuildCopyLibraryToOutput(output_project_subfix, outputdir, output_executable)

    PhysicsConfig.PreBuildCommands(outputdir)

    CommonConfig.FilterConfigurations()