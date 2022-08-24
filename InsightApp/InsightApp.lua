project "InsightApp"  
    kind "SharedLib"   
    language "C++"
    cppdialect "C++17"
    configurations { "Debug", "Release" } 

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
    debugdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")

    dependson 
    {
        "InsightCore",
        "InsightGraphics",
        "InsightECS",
    }

    defines
    {
        "IS_EXPORT_APP_DLL"
    }
    
    includedirs
    {
        "inc",
        "%{IncludeDirs.InsightCore}",
        "%{IncludeDirs.InsightGraphics}",
        "%{IncludeDirs.InsightECS}",

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
        "InsightECS.lib",
        "glm.lib",
        "tracy.lib",
    }

    libdirs
    {
        "%{wks.location}/vendor/glfw/lib",
    }

    postbuildcommands
    {
        "{COPY} \"%{cfg.targetdir}/%{prj.name}.dll\" \"%{wks.location}/deps/".. outputdir..  "/dll/\"",
        "{COPY} \"%{cfg.targetdir}/%{prj.name}.lib\" \"%{wks.location}/deps/".. outputdir..  "/lib/\"",
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