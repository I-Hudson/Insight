project "InsightCore"  
    kind "SharedLib"   
    language "C++"
    cppdialect "C++17"
    configurations { "Debug", "Release" } 

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
    debugdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")

    defines
    {
        "IS_EXPORT_CORE_DLL"
    }
    
    includedirs
    {
        "inc",
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
    }

    libdirs
    {
        "%{wks.location}/deps/lib",
    }

    postbuildcommands
    {
       "{COPY} \"%{cfg.targetdir}/InsightCore.dll\" \"%{wks.location}/bin/".. outputdir..  "/InsightEditor/\"",
    }

    filter "configurations:Debug"
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