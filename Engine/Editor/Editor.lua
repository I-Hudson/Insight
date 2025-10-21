local CommonConfig = require "../lua/CommonConfig"
local EditorConfig = require "lua/EditorConfig"

project "Insight_Editor"  
    kind "ConsoleApp"   
    configurations { "Debug", "Release" } 
    location "./"
    
    debugargs { "project_path=../../../../InsightDemoProject/InsightDemoProject.isproject" }

    targetname ("%{prj.name}")
    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
    debugdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")

    CommonConfig.LinkToInsightModule("Core")
    CommonConfig.LinkToInsightModule("Maths")
    CommonConfig.LinkToInsightModule("Physics")
    CommonConfig.LinkToInsightModule("Input")
    CommonConfig.LinkToInsightModule("Graphics")
    CommonConfig.LinkToInsightModule("Runtime")

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

        "../../vendor/ImGuizmo/GraphEditor.cpp",
        "../../vendor/ImGuizmo/GraphEditor.h",
        "../../vendor/ImGuizmo/ImCurveEdit.cpp",
        "../../vendor/ImGuizmo/ImCurveEdit.h",
        "../../vendor/ImGuizmo/ImGradient.cpp",
        "../../vendor/ImGuizmo/ImGradient.h",
        "../../vendor/ImGuizmo/ImGuizmo.cpp",
        "../../vendor/ImGuizmo/ImGuizmo.h",
        "../../vendor/ImGuizmo/ImSequencer.cpp",
        "../../vendor/ImGuizmo/ImSequencer.h",
        "../../vendor/ImGuizmo/ImZoomSlider.h",
    }

    EditorConfig.DefinesSharedLib()
    EditorConfig.IncludeDirs()
    EditorConfig.LibraryDirs()
    EditorConfig.LibraryLinks(output_project_subfix)
    EditorConfig.PostBuildCommands(output_project_subfix, outputdir)

    EditorConfig.FilterConfigurations()
    EditorConfig.FilterPlatforms()