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
        "glm",
    }

    libdirs
    {
        "%{wks.location}/vendor/glfw/lib",
    }

    prebuildcommands
    {
        --"{COPY} \"%{wks.location}/vendor/glfw/lib/glfw3.dll\" \"%{cfg.targetdir}\"",
        "{COPYDIR} \"%{wks.location}/deps/" .. outputdir .. "/dll/\" \"%{cfg.targetdir}\"",
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