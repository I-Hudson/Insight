require("premake5")

local AMD_Ryzen_Master_SDK = os.getenv("AMDRMMONITORSDKPATH")

project "Insight_Core"  
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
        "IS_EXPORT_CORE_DLL",
    }
    
    includedirs
    {
        "inc",
        "%{IncludeDirs.spdlog}",
        "%{IncludeDirs.imgui}",
        "%{IncludeDirs.xxHash}",
        "%{IncludeDirs.glm}",
        "%{IncludeDirs.lz4}",
        "%{IncludeDirs.zip}",
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
        "imgui.lib",
        "glm.lib",
        "zip.lib",
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

    filter { "kind:SharedLib or SharedLib" }
        postbuildcommands
        {
            "{COPY} \"%{cfg.targetdir}/%{prj.name}" .. output_project_subfix .. ".dll\" \"%{wks.location}deps/".. outputdir..  "/dll/\"\n",
            "{COPY} \"%{cfg.targetdir}/%{prj.name}" .. output_project_subfix .. ".lib\" \"%{wks.location}deps/".. outputdir..  "/lib/\"\n",
            "{COPY} \"%{cfg.targetdir}/%{prj.name}" .. output_project_subfix .. ".pdb\" \"%{wks.location}deps/".. outputdir..  "/pdb/\"\n",
            "{COPY} \"%{cfg.targetdir}/%{prj.name}" .. output_project_subfix .. ".dll\" \"%{wks.location}bin/".. outputdir..  "/" .. output_executable .. "/\"\n",
        }
    filter { "kind:StaticLib or StaticLib" }
        postbuildcommands
        {
            "{COPY} \"%{cfg.targetdir}/%{prj.name}" .. output_project_subfix .. ".lib\" \"%{wks.location}deps/".. outputdir..  "/lib/\"\n",
            "{COPY} \"%{cfg.targetdir}/%{prj.name}" .. output_project_subfix .. ".pdb\" \"%{wks.location}deps/".. outputdir..  "/pdb/\"\n",
        }

    filter "configurations:Debug or configurations:Testing"
        defines { "DEBUG" }  
        symbols "On" 
        links
        {
            "xxHashd.lib",
            "lz4d.lib",
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
            "xxHash.lib",
            "lz4.lib",
        }
        libdirs
        {
            "%{wks.location}/deps/lib/release",
        }

    filter "platforms:Win64"
        if AMD_Ryzen_Master_SDK == nil then
        else
            defines
            {
                "AMD_Ryzen_Master_SDK",
            }
            includedirs
            {
                AMD_Ryzen_Master_SDK .. "/include",
            }
            links
            {
                "Platform.lib",
                "Device.lib",
            }
            prebuildcommands
            {
                "{COPY} \"" .. AMD_Ryzen_Master_SDK .. "/bin/Platform.dll\" \"%{wks.location}deps/".. outputdir..  "/dll/\"\n",
                "{COPY} \"" .. AMD_Ryzen_Master_SDK .. "/lib/Platform.lib\" \"%{wks.location}deps/".. outputdir..  "/lib/\"\n",
                "{COPY} \"" .. AMD_Ryzen_Master_SDK .. "/bin/Device.dll\" \"%{wks.location}deps/".. outputdir..  "/dll/\"\n",
                "{COPY} \"" .. AMD_Ryzen_Master_SDK .. "/lib/Device.lib\" \"%{wks.location}deps/".. outputdir..  "/lib/\"\n",
            }
        end
        links
        {
            "Ole32.lib",
            "dbghelp.lib",
            "Rpcrt4.lib",
        }

    filter "configurations:Testing" 
        links
        {
        }