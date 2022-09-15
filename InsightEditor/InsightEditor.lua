project "InsightEditor"  
    kind "ConsoleApp"   
    language "C++"
    cppdialect "C++17"
    configurations { "Debug", "Release" } 

    targetname ("%{prj.name}")
    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
    debugdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")

    dependson 
    {
        "InsightApp",
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
        "%{IncludeDirs.InsightECS}",
        "%{IncludeDirs.InsightApp}",
    
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
        "InsightCore.lib",
        "InsightGraphics.lib",
        "InsightApp.lib",
        "glm.lib",
    }

    libdirs
    {
        "%{wks.location}/vendor/glfw/lib",
    }

    prebuildcommands
    {
        "{COPYDIR} \"%{wks.location}/deps/" .. outputdir .. "/dll/\" \"%{cfg.targetdir}\"",
    }

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