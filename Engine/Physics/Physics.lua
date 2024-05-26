local local_post_build_commands = post_build_commands

--- Check if a file or directory exists in this path
function fileExists(file)
    local ok, err, code = os.rename(file, file)
    if not ok then
       if code == 13 then
          --print "Permission denied, but it exists"
          return true
       elseif code == 5 then
        --print "Permission denied, but it exists"
        return true
       end
    end
    return false
 end
 
 --- Check if a directory exists in this path
 function isdir(path)
    -- "/" works on both Unix and Windows
    local result = fileExists(path)
    return result
 end

project "Insight_Physics"  
    configurations { "Debug", "Release" } 
    location "./"
    
    targetname ("%{prj.name}" .. output_project_subfix)
    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
    debugdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")

    dependson 
    { 
        "Insight_Core",
    }

    defines
    {
        "IS_EXPORT_PHYSICS_DLL",
    }
    
    includedirs
    {
        "inc",
        "%{IncludeDirs.InsightCore}",
        
        "%{IncludeDirs.spdlog}",
        "%{IncludeDirs.imgui}",
        "%{IncludeDirs.JoltPhysics}",
    }

    files 
    { 
        "inc/**.hpp", 
        "inc/**.h", 
        "inc/**.inl", 
        "src/**.cpp",
        "src/**.c",
        "src/**.inl",
        "inc/**.cpp", 

        "**.natvis",
        "../../vendor/JoltPhysics/Jolt/Jolt.natvis",
    }

    links
    {
        "Insight_Core" .. output_project_subfix .. ".lib",
        "imgui.lib",
        "Jolt.lib",
    }
    
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
        libdirs
        {
            "%{wks.location}/deps/lib/debug",
        }
        prebuildcommands
        {
            "{COPY} \"%{wks.location}vendor/JoltPhysics/Build/VS2022_CL/Debug/Jolt.lib\" \"%{wks.location}deps/".. outputdir..  "/lib/\"\n",
        }

    filter "configurations:Release"  
        defines { "NDEBUG" }    
        optimize "On" 
        libdirs
        {
            "%{wks.location}/deps/lib/release",
        }
        prebuildcommands
        {
            "{COPY} \"%{wks.location}vendor/JoltPhysics/Build/VS2022_CL/Release/Jolt.lib\" \"%{wks.location}deps/".. outputdir..  "/lib/\"\n",
        }