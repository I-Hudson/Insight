local local_post_build_commands = post_build_commands

--- Check if a file or directory exists in this path
function exists(file)
    local ok, err, code = os.rename(file, file)
    if not ok then
       if code == 13 then
          -- Permission denied, but it exists
          return true
       end
    end
    return ok, err
 end
 
 --- Check if a directory exists in this path
 function isdir(path)
    -- "/" works on both Unix and Windows
    return exists(path.."/")
 end

project "Insight_Runtime"  
    configurations { "Debug", "Release" } 
    location "./"
    
    targetname ("%{prj.name}" .. output_project_subfix)
    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
    debugdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")

    dependson 
    { 
        "Insight_Core",
        "Insight_Maths",
        "Insight_Graphics",
        "Insight_Input",
    }

    defines
    {
        "IS_EXPORT_RUNTIME_DLL"
    }
    
    includedirs
    {
        "inc",
        "%{IncludeDirs.InsightCore}",
        "%{IncludeDirs.InsightMaths}",
        "%{IncludeDirs.InsightGraphics}",
        "%{IncludeDirs.InsightInput}",

        "%{IncludeDirs.spdlog}",
        "%{IncludeDirs.glm}",
        "%{IncludeDirs.imgui}",
        "%{IncludeDirs.glfw}",
        "%{IncludeDirs.stb_image}",
        "%{IncludeDirs.splash}",
        "%{IncludeDirs.qoi}",
        "%{IncludeDirs.zip}",
        
        "%{IncludeDirs.assimp}",
        "%{IncludeDirs.assimp}/../build/include",
        "%{IncludeDirs.meshoptimizer}",
        "%{IncludeDirs.simplygon}",
        "%{IncludeDirs.reflect}",
    }

    files 
    { 
        "inc/**.hpp", 
        "inc/**.h", 
        "inc/**.inl", 
        "src/**.cpp",
        "src/**.inl",

        "inc/**.cpp", 

        "../Core/inc/Memory/NewDeleteOverload.h", 
        "../Core/src/Memory/NewDeleteOverload.cpp",

        "../../vendor/stb/stb_image.h",
        "../../vendor/stb/stb_image_write.h",
    }

    links
    {
        "Insight_Core" .. output_project_subfix .. ".lib",
        "Insight_Maths" .. output_project_subfix .. ".lib",
        "Insight_Graphics" .. output_project_subfix .. ".lib",
        "Insight_Input" .. output_project_subfix .. ".lib",
        
        "glm.lib",
        "imgui.lib",
        "zip.lib",
        "meshoptimizer.lib",
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

    filter "platforms:Win64"
        local NVIDIATextureToolsPath="C:/Program Files/NVIDIA Corporation/NVIDIA Texture Tools"      
        --if isdir(NVIDIATextureToolsPath) then
            defines
            {
                "NVIDIA_Texture_Tools",
            }
            includedirs
            {
                NVIDIATextureToolsPath .. "/include",
            }
            links
            {
                "nvtt30205.lib",
            }
            prebuildcommands
            {
                "{COPY} \"" .. NVIDIATextureToolsPath .. "/lib/x64-v142/nvtt30205.lib\" \"%{wks.location}deps/".. outputdir..  "/lib/\"\n",
                "{COPY} \"" .. NVIDIATextureToolsPath .. "/nvtt30205.dll\" \"%{wks.location}deps/".. outputdir..  "/dll/\"\n",
            }
        --end    

    filter "configurations:Debug or configurations:Testing"
        defines { "DEBUG" }  
        symbols "On" 
        links
        {
            "assimpd.lib",
            "Splashd.lib",
            "Reflectd.lib",
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
            "assimp.lib",
            "Splash.lib",
            "Reflect.lib",
        }
        libdirs
        {
            "%{wks.location}/deps/lib/release",
        }