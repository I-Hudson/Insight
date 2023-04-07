local local_post_build_commands = {}

project "Insight_Editor"  
    kind "ConsoleApp"   
    configurations { "Debug", "Release" } 
    location "./"
    
    debugargs { "project_path=../../../DemoProject/DemoProject.isproject" }

    targetname ("%{prj.name}")
    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
    debugdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")

    dependson 
    {
        "Insight_Core",
        "Insight_Maths",
        "Insight_Input",
        "Insight_Graphics",
        "Insight_Runtime",
    }

    defines
    {
        "IS_EDITOR"
    }
    
    includedirs
    {
        "inc",
        "%{IncludeDirs.InsightCore}",
        "%{IncludeDirs.InsightMaths}",
        "%{IncludeDirs.InsightInput}",
        "%{IncludeDirs.InsightGraphics}",
        "%{IncludeDirs.InsightRuntime}",
    
        "%{IncludeDirs.glfw}",
        "%{IncludeDirs.glm}",
        "%{IncludeDirs.spdlog}",
        "%{IncludeDirs.imgui}",
        "%{IncludeDirs.imgui_string}",
    }

    files 
    { 
        "inc/**.hpp", 
        "inc/**.h", 
        "inc/**.inl", 
        "src/**.cpp",
        "src/**.inl",

        "../Core/inc/Memory/NewDeleteOverload.h", 
        "../Core/src/Memory/NewDeleteOverload.cpp",
    }

    links
    {
        "Insight_Core" .. output_project_subfix .. ".lib",
        "Insight_Maths" .. output_project_subfix .. ".lib",
        "Insight_Input" .. output_project_subfix .. ".lib",
        "Insight_Graphics" .. output_project_subfix .. ".lib",
        "Insight_Runtime" .. output_project_subfix .. ".lib",

        "glm.lib",
        "imgui.lib",
    }
    if (profileTool == "pix") then
        links
        {
            "WinPixEventRuntime.lib"
        }
    end
    
    libdirs
    {
        "%{wks.location}/vendor/glfw/lib",
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
        prebuildcommands { "{COPYDIR} \"%{wks.location}deps/" .. outputdir .. "/dll/\" \"%{cfg.targetdir}\"", "{COPYDIR} \"%{wks.location}deps/" .. outputdir .. "/pdb/\" \"%{cfg.targetdir}\"",  }

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
        prebuildcommands { "{COPYDIR} \"%{wks.location}deps/" .. outputdir .. "/dll/\" \"%{cfg.targetdir}\"", "{COPYDIR} \"%{wks.location}deps/" .. outputdir .. "/pdb/\" \"%{cfg.targetdir}\"", }


    filter "configurations:Testing" 
        links
        {
            "doctest.lib",
        }

    filter "configurations:Testing"
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
        prebuildcommands { "{COPYDIR} \"%{wks.location}deps/Debug-windows-x86_64/dll\" \"%{cfg.targetdir}\"", }
        prebuildcommands 
        { 
            "{COPYDIR} \"%{wks.location}deps/" .. outputdir .. "/dll/\" \"%{cfg.targetdir}\"", 
            "{COPYDIR} \"%{wks.location}deps/" .. outputdir .. "/pdb/\" \"%{cfg.targetdir}\"", 
        }
