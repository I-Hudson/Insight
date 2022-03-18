project "InsightEditor"  
    kind "ConsoleApp"   
    language "C++"
    cppdialect "C++17"
    configurations { "Debug", "Release" } 

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
    debugdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")

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
    filter "configurations:Release"  
        defines { "NDEBUG" }    
        optimize "On" 