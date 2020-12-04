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

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Include directories relative to root folder (solution directory)
IncludeDir = {}
IncludeDir["spdlog"] = "$(SolutionDir)Insight/vendor/spdlog/include"
IncludeDir["GLFW"] = "$(SolutionDir)Insight/vendor/GLFW/include"
IncludeDir["Glad"] = "$(SolutionDir)Insight/vendor/glad/include"
IncludeDir["assimp"] = "$(SolutionDir)Insight/vendor/assimp/include"
IncludeDir["assimp_gen"] = "$(SolutionDir)Insight/vendor/assimp/BINARIES/x64/Include"
IncludeDir["Vulkan"] = os.getenv("VK_SDK_PATH") .. "/Include"
IncludeDir["glm"] = "$(SolutionDir)Insight/vendor/glm"
IncludeDir["stb_image"] = "$(SolutionDir)Insight/vendor/stb_image"
IncludeDir["imgui"] = "$(SolutionDir)Insight/vendor/imgui/"
IncludeDir["TinyXML2"] = "$(SolutionDir)Insight/vendor/TinyXML2"
IncludeDir["optick"] = "$(SolutionDir)Insight/vendor/optick/src"
IncludeDir["tinygltf"] = "$(SolutionDir)Insight/vendor/tinygltf"
IncludeDir["ktx"] = "$(SolutionDir)Insight/vendor/KTX-Software/include"

-- Lib directories relative to root folder (solution directory)
LibDirs = {}
LibDirs["Vulkan"] = "C:/VulkanSDK/1.1.130.0/lib"
LibDirs["assimp"] = "$(ProjectDir)vendor/assimp/BINARIES/x64/lib"
LibDirs["optick"] = "$(SolutionDir)Insight/vendor/optick/bin/vs2019/x64/Debug"
LibDirs["ktx"] = "$(SolutionDir)Insight/vendor/KTX-Software/build/$(Configuration)"


group "Dependencies"
    include "Insight/vendor/GLFW"
    include "Insight/vendor/Glad"
    include "Insight/vendor/ImGui"
group ""

include "Insight"
include "Sandbox"