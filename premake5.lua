workspace "Insight"
    architecture "x64"
    startproject "Sandbox"

    configurations
    {
        "Debug",
        "Release",
        "Dist"
    }
    
    flags
	{
		"MultiProcessorCompile"
    }
    
    defines
    {
        "IS_PROFILE",
    }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Include directories relative to root folder (solution directory)
IncludeDir = {}
IncludeDir["spdlog"] = "$(SolutionDir)Insight/vendor/spdlog/include"
IncludeDir["GLFW"] = "$(SolutionDir)Insight/vendor/GLFW/include"
IncludeDir["Glad"] = "$(SolutionDir)Insight/vendor/glad/include"
IncludeDir["assimp"] = "$(SolutionDir)Insight/vendor/assimp/include"
IncludeDir["assimp_gen"] = "$(SolutionDir)Insight/vendor/assimp/BINARIES/x64/Include"
IncludeDir["glm"] = "$(SolutionDir)Insight/vendor/glm"
IncludeDir["stb_image"] = "$(SolutionDir)Insight/vendor/stb_image"
IncludeDir["imgui"] = "$(SolutionDir)Insight/vendor/imgui/"
IncludeDir["SPIRV_Cross"] = "$(SolutionDir)Insight/vendor/SPIRV-Cross/"
IncludeDir["TinyXML2"] = "$(SolutionDir)Insight/vendor/TinyXML2"
IncludeDir["json"] = "$(SolutionDir)Insight/vendor/json/single_include/nlohmann"
IncludeDir["optick"] = "$(SolutionDir)Insight/vendor/optick/src"
IncludeDir["tinygltf"] = "$(SolutionDir)Insight/vendor/tinygltf"
IncludeDir["ktx"] = "$(SolutionDir)Insight/vendor/KTX-Software/include"
IncludeDir["IconsFontAwesome"] = "$(SolutionDir)Insight/vendor/IconFontCppHeaders/"

IncludeDir["Vulkan"] = os.getenv("VK_SDK_PATH") .. "/Include"
IncludeDir["glslang"] = "$(SolutionDir)Insight/vendor/glslang/"
IncludeDir["VulkanMemoryAllocator"] = "$(SolutionDir)Insight/vendor/VulkanMemoryAllocator/src"

-- Editor Includes
IncludeDir["imguiFileDialog"] = "$(SolutionDir)InsightEditor/vendor/ImGui-Addons/FileBrowser/"
IncludeDir["dirent"] = "$(SolutionDir)InsightEditor/vendor/ImGui-Addons/FileBrowser/dirent/"


-- Lib directories relative to root folder (solution directory)
LibDirs = {}
LibDirs["assimp"] = "$(ProjectDir)vendor/assimp/BINARIES/x64/lib"
LibDirs["optick"] = "$(SolutionDir)Insight/vendor/optick/bin/vs2019/x64/Debug"
LibDirs["ktx"] = "$(SolutionDir)Insight/vendor/KTX-Software/build/$(Configuration)"
LibDirs["Vulkan"] = os.getenv("VK_SDK_PATH") .. "/lib/"
LibDirs["glslang"] = "$(SolutionDir)Insight/vendor/glslang/build/"


group "Dependencies"
    include "Insight/vendor/GLFW"
    include "Insight/vendor/Glad"
    include "Insight/vendor/ImGui"
    include "Insight/vendor/SPIRV-Cross"
group ""

include "Insight"
include "InsightEditor"
include "Sandbox"