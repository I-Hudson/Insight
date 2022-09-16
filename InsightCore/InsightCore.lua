local_post_build_commands = post_build_commands

project "InsightCore"  
    --kind "SharedLib"   
    language "C++"
    cppdialect "C++17"

    targetname ("%{prj.name}" .. output_project_subfix)
    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
    debugdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")

    dependson 
    {
    }

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
        "tracy.lib",
        "OptickCore.lib",
    }

    libdirs
    {
        "%{wks.location}/deps/lib",
    }

    postbuildcommands "%{concat_table(local_post_build_commands)}"

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

    filter "platforms:Win64"
        links
        {
            "Ole32.lib",
            "dbghelp.lib",
            "Rpcrt4.lib",
        }

    filter "configurations:Testing" 
        links
        {
            "doctest.lib",
        }