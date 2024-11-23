workspace "Dependencies"
    architecture "x64"
    staticruntime "off"
    location "../../Engine/Vendor"

    configurations
    {
        "Debug",
        "Release",
    }
    platforms 
    { 
        "Win64",
    }

    flags
    {
    	"MultiProcessorCompile"
    }

    defines
    {
        "_CRT_SECURE_NO_WARNINGS",
        "GLM_FORCE_SWIZZLE",
    }

    filter "system:Windows"
        system "windows"
        toolset("msc-v143")

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
outputdir_target = "../bin/" .. outputdir
outputdir_obj = "../bin-int/" .. outputdir
outputdir_debug = "../bin/" .. outputdir
output_deps = "../../../deps/".. outputdir


IncludeDirs = {}
IncludeDirs["glfw"] = "%{wks.location}/../../vendor/glfw/include"

LibDirs = {}
LibDirs["glfw"] ="%{wks.location}/../../Vendor/glfw/lib"

VULKAN_SDK = os.getenv("VULKAN_SDK")
if VULKAN_SDK == nil then
else
    IncludeDirs["vulkan"] = VULKAN_SDK .. "/include/"
    LibDirs["vulkan"] = VULKAN_SDK .. "/lib/"
end

group "Dependices"
        include "../../premakeFiles/assimp.lua"
        include "../../premakeFiles/glfw.lua"
        include "../../premakeFiles/glm.lua"
        include "../../premakeFiles/imgui.lua"
        include "../../premakeFiles/tracy.lua"
        --include "../../premakeFiles/tracyProfiler.lua"
        --include "../../premakeFiles/doctest.lua"
        include "../../premakeFiles/meshoptimizer.lua"
        include "../../premakeFiles/xxHash.lua"
        include "../../premakeFiles/lz4.lua"
        include "../../premakeFiles/splash.lua"
        include "../../premakeFiles/spdlog.lua"
        include "../../premakeFiles/reflect.lua"
        include "../../premakeFiles/zip.lua"
        include "../../premakeFiles/efsw.lua"

