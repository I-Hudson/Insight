local local_post_build_commands = {}

project "Insight_Editor"  
    kind "ConsoleApp"   
    language "C++"
    cppdialect "C++17"
    configurations { "Debug", "Release" } 
    location "./"
    
    targetname ("%{prj.name}")
    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
    debugdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")

    dependson 
    {
        "Insight_Core",
        "Insight_Graphics",
        "Insight_Runtime",
    }

    defines
    {
        ""
    }
    
    includedirs
    {
        "inc",
        "%{IncludeDirs.InsightCore}",
        "%{IncludeDirs.InsightGraphics}",
        "%{IncludeDirs.InsightRuntime}",
    
        "%{IncludeDirs.glfw}",
        "%{IncludeDirs.glm}",
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
        "Insight_Runtime.lib",

        "glm.lib",
    }

    libdirs
    {
        "%{wks.location}/vendor/glfw/lib",
    }

    prebuildcommands { "{COPYDIR} \"%{wks.location}deps/" .. outputdir .. "/dll/\" \"%{cfg.targetdir}\"", }

    filter "configurations:Debug or configurations:Testing"
        defines { "DEBUG" }  
        symbols "On"
        links
        {
            "OptickCore.lib",
        }
        libdirs
        {
            "%{wks.location}/deps/lib/debug",
        }

    filter "configurations:Release"  
        defines { "NDEBUG" }    
        optimize "On" 
        links
        {
            "OptickCore.lib",
        }
        libdirs
        {
            "%{wks.location}/deps/lib/release",
        }

    filter "configurations:Testing" 
        links
        {
            "doctest.lib",
        }