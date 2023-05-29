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
        "IS_EDITOR_ENABLED",
        "IS_EXPORT_EDITOR_DLL",
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
        "%{IncludeDirs.reflect}",
    }

    files 
    { 
        "inc/**.hpp", 
        "inc/**.h", 
        "inc/**.inl", 
        "src/**.cpp",
        "src/**.inl",

        "inc/**.cpp",

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

    postbuildcommands
    {
        "{COPY} \"%{cfg.targetdir}/%{prj.name}" .. output_project_subfix .. ".lib\" \"%{wks.location}deps/".. outputdir..  "/lib/\"\n",
        "{COPY} \"%{cfg.targetdir}/%{prj.name}" .. output_project_subfix .. ".pdb\" \"%{wks.location}deps/".. outputdir..  "/pdb/\"\n",
    }

    prebuildcommands 
    {
        --"call $(SolutionDir)Build/Engine/RunInsightReflectTool.bat"
    }

    filter "configurations:Debug"
        defines { "DEBUG" }  
        symbols "On"
        links
        {
            "Reflectd.lib",
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
            "Reflect.lib",
        }
        libdirs
        {
            "%{wks.location}/deps/lib/release",
        }
        prebuildcommands { "{COPYDIR} \"%{wks.location}deps/" .. outputdir .. "/dll/\" \"%{cfg.targetdir}\"", "{COPYDIR} \"%{wks.location}deps/" .. outputdir .. "/pdb/\" \"%{cfg.targetdir}\"", }


    filter "configurations:Testing" 
        links
        {
        }

    filter "configurations:Testing"
        defines { "DEBUG" }  
        symbols "On"
        links
        {
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
