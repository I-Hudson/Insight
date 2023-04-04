local profileTool="tracy"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
outputdir_target = "%{wks.location}bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
outputdir_obj = "%{wks.location}bin-int/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
outputdir_debug = "%{wks.location}bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
output_project_subfix = ""
output_executable = ""
output_executable = "Insight_Standalone"


VULKAN_SDK = os.getenv("VULKAN_SDK")

IncludeDirs = {}
IncludeDirs["InsightCore"] = "%{wks.location}Engine/Core/inc"
IncludeDirs["InsightMaths"] = "%{wks.location}Engine/Maths/inc"
IncludeDirs["InsightGraphics"] = "%{wks.location}Engine/Graphics/inc"
IncludeDirs["InsightInput"] = "%{wks.location}Engine/Input/inc"
IncludeDirs["InsightRuntime"] = "%{wks.location}Engine/Runtime/inc"
IncludeDirs["InsightEditor"] = "%{wks.location}Engine/Editor/inc"

IncludeDirs["doctest"] = "%{wks.location}vendor/doctest/doctest"
IncludeDirs["glfw"] = "%{wks.location}vendor/glfw/include"
IncludeDirs["glm"] = "%{wks.location}vendor/glm"
IncludeDirs["imgui"] = "%{wks.location}vendor/imgui"
IncludeDirs["implot"] = "%{wks.location}vendor/implot"
IncludeDirs["spdlog"] = "%{wks.location}vendor/spdlog/include"
IncludeDirs["vma"] = "%{wks.location}vendor/VulkanMemoryAllocator/src"
IncludeDirs["vulkan"] = VULKAN_SDK .. "/include/"
IncludeDirs["spirv_reflect"] = "%{wks.location}vendor/SPIRV-Reflect"
IncludeDirs["assimp"] = "%{wks.location}vendor/assimp/include"
IncludeDirs["optick"] = "%{wks.location}vendor/optick/src"
IncludeDirs["tracy"] = "%{wks.location}vendor/tracy"
IncludeDirs["pix"] = "%{wks.location}vendor/winpixeventruntime/Include"
IncludeDirs["stb_image"] = "%{wks.location}vendor/stb"
IncludeDirs["meshoptimizer"] = "%{wks.location}vendor/meshoptimizer/src"
IncludeDirs["FFR2"] = "%{wks.location}vendor/FidelityFX-FSR2/src"
IncludeDirs["Aftermath"] = "%{wks.location}vendor/NVIDIA_Nsight_Aftermath_SDK_2022.1.0.22080/include"
IncludeDirs["xxHash"] = "%{wks.location}vendor/xxHash"
IncludeDirs["splash"] = "%{wks.location}vendor/Splash/Splash/inc"
IncludeDirs["DirectXHeaders"] = "%{wks.location}vendor/DirectX-Headers/include/directx"
IncludeDirs["DirectXAgilityHeaders"] = "%{wks.location}vendor/Microsoft.Direct3D.D3D12/build/native/include"
IncludeDirs["DirectXShaderCompiler"] = "%{wks.location}vendor/DirectXShaderCompiler/inc"
IncludeDirs["D3D12MemoryAllocator"] = "%{wks.location}vendor/D3D12MemoryAllocator/include"
IncludeDirs["IconFontCppHeaders"] = "%{wks.location}vendor/IconFontCppHeaders"
IncludeDirs["nlohmann_json"] = "%{wks.location}vendor/nlohmann_json/single_include"

LibDirs = {}
LibDirs["deps_lib"] = "%{wks.location}deps/" .. outputdir .. "/lib/"
LibDirs["deps_testing_lib"] = "%{wks.location}deps/Debug-windows-x86_64/lib/"

LibDirs["imgui"] = "%{wks.location}vendor/imgui/" .. outputdir .. "ImGui/"
LibDirs["vulkan"] = VULKAN_SDK .. "/lib/"

workspace "InsightStandalone"
    architecture "x64"
    startproject "InsightStandalone"
    staticruntime "on"
    location "../../"

    language ("C++")
    cppdialect ("C++17")

    configurations
    {
        "Debug",
        "Release",
        "Testing",
    }
    platforms 
    { 
        "Win64", 
        "Linux" 
    }

    flags
    {
    	"MultiProcessorCompile"
    }

    includedirs
    {

    }

    if (profileTool == "tracy") then
        defines { "IS_PROFILE_ENABLED", "IS_PROFILE_TRACY", "TRACY_IMPORTS", "TRACY_ON_DEMAND", "TRACY_CALLSTACK", }
        includedirs { "%{IncludeDirs.tracy}", }
    end
    if (profileTool == "optick") then
        defines { "IS_PROFILE_ENABLED", "IS_PROFILE_OPTICK" }
        includedirs { "%{IncludeDirs.optick}", }
    end
    if (profileTool == "pix") then
        defines { "IS_PROFILE_ENABLED", "IS_PROFILE_PIX", "USE_PIX" }
        includedirs { "%{IncludeDirs.pix}", }
    end 

    libdirs
    {
        "%{LibDirs.deps_lib}",
    }

    if (profileTool == "pix") then
        links
        {
            "WinPixEventRuntime.lib"
        }
    end

include "../../Engine/Standalone/premake.lua"

newaction{
    trigger = "clean",
    description = "Remove all binaries and intermediate binaries, and vs files.",
    execute = function()
        print("Removing symlinks")
        os.execute("Remove_Symlinks.bat")
        print("Removeing binaries")
        os.rmdir("../../bin")
        print("Removeing internediate binaries")
        os.rmdir("../../bin-int")
        print("Removing dependencies")
        os.rmdir("../../deps")
        print("Removeing project files")
        os.rmdir("../../.vs")
        os.remove("../../Engine/**.sln")
        os.remove("../../Engine/**.vcxproj")
        os.remove("../../Engine/**.vcxproj.filters")
        os.remove("../../Engine/**.vcxproj.user")
        print("Done")
    end
} 