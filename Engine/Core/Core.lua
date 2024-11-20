require("premake5")

local AMD_Ryzen_Master_SDK = os.getenv("AMDRMMONITORSDKPATH")

local CommonConfig = require "../lua/CommonConfig"
local CoreConfig = require "lua/CoreConfig"

project "Insight_Core"  
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

    CoreConfig.DefinesSharedLib()
    
    CoreConfig.IncludeDirs()
    CoreConfig.LibraryDirs()
    CoreConfig.LibraryLinks()

    CoreConfig.FilterConfigurations()
    CoreConfig.FilterPlatforms(AMD_Ryzen_Master_SDK, outputdir)

    CommonConfig.PostBuildCopyLibraryToOutput(output_project_subfix, outputdir, output_executable)