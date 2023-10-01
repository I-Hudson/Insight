project "AssetPacker"  
    configurations { "Debug", "Release" } 
    location "./"
    kind "ConsoleApp"

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
    }

    files 
    { 

        "inc/**.hpp", 
        "inc/**.h", 
        "inc/**.inl", 
        "src/**.cpp",
        "src/**.inl",
    }

    filter "system:Windows"

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
        prebuildcommands { "{COPYDIR} \"%{wks.location}deps/" .. outputdir .. "/dll/Reflectd.dll\" \"%{cfg.targetdir}\"" }


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
        prebuildcommands { "{COPYDIR} \"%{wks.location}deps/" .. outputdir .. "/dll/Reflect.dll\" \"%{cfg.targetdir}\"" }


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
