local local_post_build_commands = post_build_commands

project "Runtime"  
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
        "InsightCore",
        "InsightGraphics",
        "InsightInput",
    }

    defines
    {
        "IS_EXPORT_RUNTIME_DLL"
    }
    
    includedirs
    {
        "inc",
        "%{IncludeDirs.InsightCore}",
        "%{IncludeDirs.InsightGraphics}",
        "%{IncludeDirs.InsightInput}",

        "%{IncludeDirs.spdlog}",
        "%{IncludeDirs.optick}",
        "%{IncludeDirs.glm}",
        "%{IncludeDirs.imgui}",
        "%{IncludeDirs.glfw}",
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
        "InsightGraphics.lib",
        "InsightInput.lib",
        
        "glm.lib",
        "imgui.lib",
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

    filter "configurations:Testing" 
        links
        {
            "doctest.lib",
        }