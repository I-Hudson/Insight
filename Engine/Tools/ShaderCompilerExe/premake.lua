project "ShaderCompilerExe"  
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

    dependson
    {
        "ShaderCompiler"
    }

    includedirs
    {
        "inc",
        "%{IncludeDirs.InsightToolShaderCompiler}",
    }

    libdirs
    {
        "%{wks.location}/deps/lib",
    }

    files 
    { 

        "inc/**.hpp", 
        "inc/**.h", 
        "inc/**.inl", 
        "src/**.cpp",
        "src/**.inl",
    }

    links
    {
        "ShaderCompiler",
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
