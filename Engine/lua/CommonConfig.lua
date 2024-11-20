local CommonConfig = { }

function CommonConfig.PostBuildCopyLibraryToOutput()
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
end

function CommonConfig.FilterConfigurations()
    filter "configurations:Debug or configurations:Testing"
        defines { "DEBUG" }  
        symbols "On" 
        libdirs
        {
            "%{wks.location}/deps/lib/debug",
        }

    filter "configurations:Release"
        defines { "NDEBUG" }    
        optimize "On" 
        libdirs
        {
            "%{wks.location}/deps/lib/release",
        }
end

return CommonConfig