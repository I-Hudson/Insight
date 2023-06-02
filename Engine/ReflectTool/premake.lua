project "InsightReflectTool"  
    configurations { "Debug", "Release" } 
    location "./"
    kind "ConsoleApp"

    targetname ("%{prj.name}" .. output_project_subfix)
    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
    debugdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")

    defines
    {
        "REFLECT_TYPE_INFO_ENABLED",
    }

    includedirs
    {
        "inc",
        "%{IncludeDirs.insight_editor}",
        "%{IncludeDirs.reflect}",
    }

    files 
    { 

        "inc/**.hpp", 
        "inc/**.h", 
        "inc/**.inl", 
        "src/**.cpp",
        "src/**.inl",

        "../Editor/inc/Editor/HotReload/HotReloadExportFunctions.h.h", 
    }

    filter "system:Windows"

    links
    {
        "tracy.lib",
    }

    filter "configurations:Debug or configurations:Testing"
        buildoptions "/MDd"
        defines
        {
            "_DEBUG",
            "IS_DEBUG",
        }
        links
        {
            "Reflectd.lib",
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
            "Reflect.lib",
        }

    filter "system:Windows"
    	system "windows"
    	toolset("msc-v143")
        defines
        {
        }
        includedirs
         { 
            "%{IncludeDirs.pix}",
         }
        links
        {
            "WinPixEventRuntime.lib",
        }

    filter "system:Unix"
    	system "linux"
    	toolset("clang")
        defines
        {
            "IS_PLATFORM_LINUX",
            "IS_VULKAN_ENABLED",
        }
