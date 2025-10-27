project "InsightReflectTool"  
    configurations { "Debug", "Release" } 
    location "./"
    kind "ConsoleApp"

    targetname ("%{prj.name}" .. output_project_subfix)
    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
    debugdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")

    debugargs { "Type=Engine ReflectPath=../../../Engine/ GenerateProjectFileOutputPath=../../../Engine" }

    defines
    {
        "REFLECT_TYPE_INFO_ENABLED",
    }

    includedirs
    {
        "inc",
        "%{IncludeDirs.insight_editor}",
        "%{IncludeDirs.InsightRuntime}",
        "%{IncludeDirs.reflect}",
    }

    files 
    { 

        "inc/**.hpp", 
        "inc/**.h", 
        "inc/**.inl", 
        "src/**.cpp",
        "src/**.inl",

        "../Editor/inc/Editor/HotReload/HotReloadExportFunctions.h", 
    }

    filter "system:Windows"

    links
    {
        "TracyClient",
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
            "Reflectd",
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
            "Reflect",
        }
        prebuildcommands { "{COPYDIR} \"%{wks.location}deps/" .. outputdir .. "/dll/Reflect.dll\" \"%{cfg.targetdir}\"" }
