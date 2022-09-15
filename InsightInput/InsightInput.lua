project "InsightInput"  
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
    }

    defines
    {
        "IS_EXPORT_INPUT_DLL",
    }
    
    includedirs
    {
        "inc",
        "%{IncludeDirs.InsightCore}",
        "%{IncludeDirs.InsightGraphics}",

        "%{IncludeDirs.glfw}",
        "%{IncludeDirs.glm}",
        "%{IncludeDirs.imgui}",
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
        "GLFW.lib",
        "glm.lib",
        "imgui.lib",
    }

    libdirs
    {
    }

    postbuildcommands
    {
        "{COPY} \"%{cfg.targetdir}/%{prj.name}.dll\" \"%{wks.location}/deps/".. outputdir..  "/dll/\"",
        "{COPY} \"%{cfg.targetdir}/%{prj.name}.lib\" \"%{wks.location}/deps/".. outputdir..  "/lib/\"",

        "{COPY} \"%{cfg.targetdir}/%{prj.name}.dll\" \"%{wks.location}/bin/".. outputdir..  "/" .. output_executable .. "\"",
    }

    filter "configurations:Debug or configurations:Testing"
        defines { "DEBUG" }
        symbols "On" 
        links
        {
        }

    filter "configurations:Release"  
        defines { "NDEBUG" }    
        optimize "On"   
        links
        {
        }

    filter "configurations:Testing" 
        links
        {
            "doctest.lib",
        }
        libdirs
        {
            "%{LibDirs.deps_testing_lib}",
        }