local_post_build_commands = post_build_commands

project "Insight_Maths"  
    language "C++"
    cppdialect "C++17"
    location "./"

    targetname ("%{prj.name}" .. output_project_subfix)
    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
    debugdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")

    dependson 
    {
    }

    defines
    {
        "IS_EXPORT_MATHS_DLL"
    }
    
    includedirs
    {
        "inc",
        "%{IncludeDirs.spdlog}",
        "%{IncludeDirs.optick}",
        "%{IncludeDirs.imgui}",
    }

    files 
    { 
        "inc/**.hpp", 
        "inc/**.h", 
        "src/**.cpp" 
    }

    links
    {
        "imgui.lib",
        "tracy.lib",
        "OptickCore.lib",
    }
    if (profileTool == "pix") then
        links
        {
            "WinPixEventRuntime.lib"
        }
    end
    
    libdirs
    {
        "%{wks.location}/deps/lib",
    }

    --postbuildcommands "%{concat_table(local_post_build_commands)}"
    postbuildcommands
    {
        "{COPY} \"%{cfg.targetdir}/%{prj.name}" .. output_project_subfix .. ".dll\" \"%{wks.location}deps/".. outputdir..  "/dll/\"\n",
        "{COPY} \"%{cfg.targetdir}/%{prj.name}" .. output_project_subfix .. ".lib\" \"%{wks.location}deps/".. outputdir..  "/lib/\"\n",
        "{COPY} \"%{cfg.targetdir}/%{prj.name}" .. output_project_subfix .. ".pdb\" \"%{wks.location}deps/".. outputdir..  "/pdb/\"\n",
        "{COPY} \"%{cfg.targetdir}/%{prj.name}" .. output_project_subfix .. ".dll\" \"%{wks.location}bin/".. outputdir..  "/" .. output_executable .. "/\"\n",
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

    filter "platforms:Win64"
        links
        {
            "Ole32.lib",
            "dbghelp.lib",
            "Rpcrt4.lib",
        }

    filter "configurations:Testing" 
        links
        {
            "doctest.lib",
        }
         