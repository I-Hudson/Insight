project "InsightEditor"  
    kind "ConsoleApp"   
    language "C++"
    cppdialect "C++17"
    configurations { "Debug", "Release" } 

    targetdir ("%{wks.location}" .. outputdir_target .. "/%{prj.name}")
    objdir ("%{wks.location}" .. outputdir_obj .. "/%{prj.name}")
    debugdir ("%{wks.location}" .. outputdir_debug .. "/%{prj.name}")

    defines
    {
        ""
    }
    
    includedirs
    {
        "inc",
        "%{IncludeDirs.InsightCore}",
        "%{IncludeDirs.InsightGraphics}",
        "%{IncludeDirs.InsightApp}",
    
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
        "InsightCore",
        "InsightGraphics",
        "InsightApp",

        "glfw3.lib",
        "glm",
    }

    libdirs
    {
        "%{wks.location}/vendor/glfw/lib",
    }

    postbuildcommands
    {
        "{COPY} \"%{wks.location}/vendor/glfw/lib/glfw3.dll\" \"%{cfg.targetdir}\"",
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
        postbuildcommands
        {
            "{COPYDIR} \"%{wks.location}/deps/dll/debug/\" \"%{cfg.targetdir}\"",
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
        postbuildcommands
        {
            "{COPYDIR} \"%{wks.location}/deps/dll/release/\" \"%{cfg.targetdir}\"",
        }