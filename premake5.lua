workspace "Insight"
    architecture "x64"
    startproject "Sandbox"

    configurations
    {
        "Debug",
        "Debug_NO_EDITOR",
        "Release",
        "Release_NO_EDITOR",

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
IncludeDir["assimp"] = "$(SolutionDir)Insight/vendor/assimp/include"
IncludeDir["shaderc"] = "$(SolutionDir)Insight/vendor/shaderc/include"
IncludeDir["Vulkan"] = "C:/VulkanSDK/1.1.130.0/Include"
IncludeDir["glm"] = "$(SolutionDir)Insight/vendor/glm"
IncludeDir["stb_image"] = "$(SolutionDir)Insight/vendor/stb_image"
IncludeDir["glad"] = "$(SolutionDir)Insight/vendor/glad/include"
IncludeDir["imgui"] = "$(SolutionDir)Insight/vendor/imgui/"
IncludeDir["json"] = "$(SolutionDir)Insight/vendor/nlohmann/include"

-- Lib directories relative to root folder (solution directory)
LibDirs = {}
LibDirs["Vulkan"] = "C:/VulkanSDK/1.1.130.0/lib"
LibDirs["assimp"] = "$(ProjectDir)vendor/assimp/lib"

group "Dependencies"
    include "Insight/vendor/GLFW"

group ""

project "Insight"
    location "Insight"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
	staticruntime "on"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
    debugdir ("bin/" .. outputdir .. "/%{prj.name}")

    pchheader "ispch.h"
    pchsource "Insight/src/ispch.cpp"

    files
	{
		"%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/vendor/stb_image/**.h",
		"%{prj.name}/vendor/stb_image/**.cpp",
        "%{prj.name}/vendor/glm/glm/**.hpp",
		"%{prj.name}/vendor/glm/glm/**.inl",
		"%{prj.name}/vendor/glad/src/**.cpp",
		"%{prj.name}/vendor/imgui/**.h",
		"%{prj.name}/vendor/imgui/**.cpp",
	}

    includedirs 
    {
		"$(ProjectDir)src",
		"%{IncludeDir.spdlog}",
        "%{IncludeDir.GLFW}",
        "%{IncludeDir.assimp}",
        "%{IncludeDir.shaderc}",
        "%{IncludeDir.Vulkan}",
        "%{IncludeDir.glm}",
        "%{IncludeDir.stb_image}",
        "%{IncludeDir.glad}",
        "%{IncludeDir.imgui}",
        "%{IncludeDir.json}",
	}

    sysincludedirs 
    {
        "%{IncludeDir.glm}",
	}

    libdirs
    {
        "%{LibDirs.Vulkan}",
        "%{LibDirs.assimp}"
	}

    links 
	{ 
        "GLFW",
        "assimp",
        "shaderc_combined",
        "vulkan-1",
        "opengl32"
	}

    filter "system:windows"
        systemversion "latest"

        defines 
        {  
            "IS_PLATFORM_WINDOWS", 
            "IS_BUILD_DLL" ,
            "GLM_FORCE_SWIZZLE"
        }

        prebuildcommands
        {
            ("{COPY} ../PBC/ ../bin/" .. outputdir .. "/Sandbox")
		}

        postbuildcommands 
        { 
            ("{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/Sandbox"),
            ("{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/UnitTests")
        }

    filter "configurations:Debug"
        defines { "IS_DEBUG", "IS_VULKAN", "IS_PROFILE", "NOMINMAX", "IS_EDITOR", "IMGUI_ENABLED", "GLFW_INCLUDE_VULKAN" }
        symbols "on"
        libdirs
        {
            "%{LibDirs.Vulkan}/Debug"
        }

    filter "configurations:Debug_NO_EDITOR"
        defines { "IS_DEBUG", "IS_VULKAN", "IS_PROFILE", "NOMINMAX", "GLFW_INCLUDE_VULKAN" }
        symbols "on"
        libdirs
        {
            "%{LibDirs.Vulkan}/Debug"
        }

    filter "configurations:Release"
        defines { "IS_RELEASE", "IS_VULKAN", "IS_PROFILE", "NOMINMAX", "IS_EDITOR", "IMGUI_ENABLED", "GLFW_INCLUDE_VULKAN" }
        optimize "on"
        libdirs
        {
            "%{LibDirs.Vulkan}/Release"

        }

    filter "configurations:Release_NO_EDITOR"
        defines { "IS_RELEASE", "IS_VULKAN", "IS_PROFILE", "NOMINMAX", "GLFW_INCLUDE_VULKAN" }
        optimize "on"
        libdirs
        {
            "%{LibDirs.Vulkan}/Release"

        }

    filter "configurations:Dist"
        defines { "IS_DIST", "IS_VULKAN", "IS_PROFILE", "NOMINMAX", "GLFW_INCLUDE_VULKAN" }
        optimize "full"
        libdirs
        {
            "%{LibDirs.Vulkan}/Release"

        }

    filter { "system:windows", "configurations:Release" }
        buildoptions "/MT"

project "Sandbox"
    location "Sandbox"
    language "C++"
    cppdialect "C++17"
	staticruntime "on"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
    debugdir ("bin/" .. outputdir .. "/%{prj.name}")

    files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

    includedirs
    {
        "$(SolutionDir)Insight/src",
        "%{IncludeDir.spdlog}",
        "%{IncludeDir.assimp}",
        "%{IncludeDir.glm}",
        "%{IncludeDir.imgui}",
        "%{IncludeDir.json}",
	}

    sysincludedirs 
    {
        "%{IncludeDir.glm}",
	}

    links
    {
        "Insight"
	}

    filter "system:windows"
        cppdialect "C++17"
        staticruntime "On"
        systemversion "latest"

        defines 
        { 
            "IS_PLATFORM_WINDOWS" 
        }


    filter "configurations:Debug"
        defines { "IS_DEBUG", "IS_VULKAN", "IS_PROFILE", "NOMINMAX", "IS_EDITOR", "IMGUI_ENABLED", "GLFW_INCLUDE_VULKAN" }
        symbols "on"
        kind "ConsoleApp"

    filter "configurations:Debug_NO_EDITOR"
        defines { "IS_DEBUG", "IS_VULKAN", "IS_PROFILE", "NOMINMAX", "GLFW_INCLUDE_VULKAN" }
        symbols "on"
        kind "ConsoleApp"

    filter "configurations:Release"
        defines { "IS_RELEASE", "IS_VULKAN", "IS_PROFILE", "NOMINMAX", "IS_EDITOR", "IMGUI_ENABLED", "GLFW_INCLUDE_VULKAN" }
        optimize "on"
        kind "WindowedApp"

    filter "configurations:Release_NO_EDITOR"
        defines { "IS_RELEASE", "IS_VULKAN", "IS_PROFILE", "NOMINMAX", "GLFW_INCLUDE_VULKAN" }
        optimize "on"
        kind "WindowedApp"

    filter "configurations:Dist"
        defines { "IS_DIST", "IS_VULKAN", "IS_PROFILE", "NOMINMAX", "GLFW_INCLUDE_VULKAN" }
        optimize "full"
        kind "WindowedApp"

        filter { "system:windows", "configurations:Release" }
            buildoptions "/MT"

project "UnitTests"
    location "UnitTests"
     kind "SharedLib"
    language "C++"
    cppdialect "C++17"
	staticruntime "on"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

    includedirs
    {
        "$(SolutionDir)Insight/src",
        "%{IncludeDir.spdlog}",
        "%{IncludeDir.assimp}",
        "%{IncludeDir.glm}",
        "%{IncludeDir.json}",
	}

    sysincludedirs 
    {
        "%{IncludeDir.glm}",
	}

    links
    {
        "Insight"
	}

    filter "system:windows"
        cppdialect "C++17"
        staticruntime "On"
        systemversion "latest"

        defines 
        { 
            "IS_PLATFORM_WINDOWS" 
        }

    filter "configurations:Debug"
        defines { "IS_DEBUG", "IS_VULKAN", "IS_PROFILE", "NOMINMAX", "IS_EDITOR", "IMGUI_ENABLED", "GLFW_INCLUDE_VULKAN" }
        symbols "on"

    filter "configurations:Debug_NO_EDITOR"
        defines { "IS_DEBUG", "IS_VULKAN", "IS_PROFILE", "NOMINMAX", "GLFW_INCLUDE_VULKAN" }
        symbols "on"