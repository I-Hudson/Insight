local CommonConfig = require "../../lua/CommonConfig"
local GraphicsConfig = require "../../Graphics/lua/GraphicsConfig"

project "ShaderCompiler"  
    configurations { "Debug", "Release" } 
    location "./"
    kind "StaticLib"

    targetname ("%{prj.name}" .. output_project_subfix)
    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
    debugdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")

    defines
    {
    }

    includedirs
    {
        "inc",
        "%{IncludeDirs.InsightCore}",
        "%{IncludeDirs.InsightGraphics}",
    }
    GraphicsConfig.IncludeDirs()

    files 
    { 

        "inc/**.hpp", 
        "inc/**.h", 
        "inc/**.inl", 
        "src/**.cpp",
        "src/**.inl",
    }

    libdirs
    {
        "%{wks.location}/deps/lib",
    }

    links
    {
        "Insight_Core.lib",
        "dxcompiler.lib",
    }

    CommonConfig.PostBuildCopyLibraryToOutput()

    filter "configurations:Debug or configurations:Testing"
        buildoptions "/MDd"
        defines
        {
            "_DEBUG",
            "IS_DEBUG",
        }
        links
        {
        }


    filter "configurations:Release"
    buildoptions "/MD"
        optimize "On"   
                defines
        {
            "NDEBUG",
            "IS_RELEASE",
            "DOCTEST_CONFIG_DISABLE",
        }
        links
        {
        }


    filter "system:Windows"
    	system "windows"
    	toolset("msc-v143")

    filter "system:Unix"
    	system "linux"
    	toolset("clang")
        defines
        {
            "IS_PLATFORM_LINUX",
            "IS_VULKAN_ENABLED",
        }
