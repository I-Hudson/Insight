local local_post_build_commands = post_build_commands

project "Insight_Input"  
    language "C++"
    cppdialect "C++17"
    configurations { "Debug", "Release" } 
    location "./"
    
    targetname ("%{prj.name}" .. output_project_subfix)
    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
    debugdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")

    dependson 
    {
        "Insight_Core",
        "Insight_Graphics",
    }

    defines
    {
        "IS_EXPORT_INPUT_DLL",
    }
    
    includedirs
    {
        "inc",
        "%{IncludeDirs.InsightCore}",
        "%{IncludeDirs.InsightGraphics}",

        "%{IncludeDirs.glfw}",
        "%{IncludeDirs.glm}",
        "%{IncludeDirs.imgui}",
        "%{IncludeDirs.spdlog}",
    }

    files 
    { 
        "inc/**.hpp", 
        "inc/**.h", 
        "src/**.cpp",
    }

    links
    {
        "Insight_Core.lib",
        "Insight_Graphics.lib",

        "GLFW.lib",
        "glm.lib",
        "imgui.lib",
    }

    libdirs
    {
    }

    postbuildcommands "%{concat_table(local_post_build_commands)}"

    filter "configurations:Debug or configurations:Testing"
        defines { "DEBUG" }
        symbols "On" 
        links
        {
        }

    filter "configurations:Release"  
        defines { "NDEBUG" }    
        optimize "On"   
        links
        {
        }

    filter "configurations:Testing" 
        links
        {
            "doctest.lib",
        }
        libdirs
        {
            "%{LibDirs.deps_testing_lib}",
        }