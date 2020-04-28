workspace "Insight"
    architecture "x64"
    startproject "Sandbox"

    configurations
    {
        "Debug",
        "Release",
        "Dist"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Include directories relative to root folder (solution directory)
IncludeDir = {}
IncludeDir["spdlog"] = "$(SolutionDir)/Insight/vendor/spdlog/include"
IncludeDir["GLFW"] = "$(SolutionDir)/Insight/vendor/GLFW/include"
IncludeDir["shaderc"] = "$(SolutionDir)/Insight/vendor/shaderc/include"
IncludeDir["Vulkan"] = "C:/VulkanSDK/1.1.130.0/Include"

-- Lib directories relative to root folder (solution directory)
LibDirs = {}
LibDirs["shaderc"] = "C:/VulkanSDK/1.1.130.0/lib"

group "Dependencies"
    include "Insight/vendor/GLFW"

group ""

project "Insight"
    location "Insight"
    kind "SharedLib"
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
		"%{prj.name}/src/**.cpp"
	}

    includedirs 
	{
		"$(ProjectDir)src",
		"%{IncludeDir.spdlog}",
        "%{IncludeDir.GLFW}",
        "%{IncludeDir.shaderc}",
        "%{IncludeDir.Vulkan}"
	}

    libdirs
    {
        "%{LibDirs.shaderc}/%{cfg.buildcfg}"
    }

    links 
	{ 
        "GLFW",
        "shaderc_combined",
        "C:/VulkanSDK/1.1.130.0/Lib/vulkan-1.lib"
	}

    filter "system:windows"
        systemversion "latest"

        defines 
        {  
            "IS_PLATFORM_WINDOWS", 
            "IS_BUILD_DLL" 
        }

        postbuildcommands 
        { 
            ("{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/Sandbox"),
            ("{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/UnitTests")
        }

    filter "configurations:Debug"
        defines "IS_DEBUG"
        symbols "on"


    filter "configurations:Release"
        defines "IS_RELEASE"
        optimize "on"


    filter "configurations:Dist"
        defines "IS_DIST"
        optimize "on"

        filter { "system:windows", "configurations:Release" }
            buildoptions "/MT"

project "Sandbox"
    location "Sandbox"
    kind "ConsoleApp"
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
        "%{IncludeDir.spdlog}"
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
        defines "IS_DEBUG"
        symbols "On"

    filter "configurations:Release"
        defines "IS_RELEASE"
        optimize "On"

    filter "configurations:Dist"
        defines "IS_DIST"
        optimize "On"

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
        "%{IncludeDir.spdlog}"
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
        defines "IS_DEBUG"
        symbols "On"