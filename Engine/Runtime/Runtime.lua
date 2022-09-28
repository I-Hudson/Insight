local local_post_build_commands = post_build_commands

project "Insight_Runtime"  
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
        "Insight_Input",
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
        "%{IncludeDirs.stb_image}",
    }

    files 
    { 
        "inc/**.hpp", 
        "inc/**.h", 
        "src/**.cpp",

        "../../vendor/stb/stb_image.h",
        "../../vendor/stb/stb_image_write.h",
    }

    links
    {
        "Insight_Core.lib",
        "Insight_Graphics.lib",
        "Insight_Input.lib",
        
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