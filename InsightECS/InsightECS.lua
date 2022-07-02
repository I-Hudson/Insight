project "InsightECS"  
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
    }

    defines
    {
        "IS_EXPORT_ECS_DLL"
    }
    
    includedirs
    {
        "inc",
        "%{IncludeDirs.InsightCore}",
        "%{IncludeDirs.spdlog}",
        "%{IncludeDirs.optick}",
    }

    files 
    { 
        "inc/**.hpp", 
        "inc/**.h", 
        "src/**.cpp" 
    }

    links
    {
        "InsightCore.lib",
        "glm.lib",
        "tracy.lib",
        "OptickCore.lib",
    }

    libdirs
    {
        "%{wks.location}/deps/lib",
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