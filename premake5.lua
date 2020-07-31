workspace "Insight"
    architecture "x64"
    startproject "Sandbox"

    configurations
    {
        "Debug_VULKAN",
        "Debug_Vulkan_SP",
        "Debug_OPENGL",
        "Debug_OPENGL_SP",
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
IncludeDir["assimp"] = "$(SolutionDir)Insight/vendor/assimp/include"
IncludeDir["shaderc"] = "$(SolutionDir)Insight/vendor/shaderc/include"
IncludeDir["Vulkan"] = "C:/VulkanSDK/1.1.130.0/Include"
IncludeDir["glm"] = "$(SolutionDir)Insight/vendor/glm"
IncludeDir["stb_image"] = "$(SolutionDir)Insight/vendor/stb_image"
IncludeDir["glad"] = "$(SolutionDir)Insight/vendor/glad/include"
IncludeDir["imgui"] = "$(SolutionDir)Insight/vendor/imgui/"

-- Lib directories relative to root folder (solution directory)
LibDirs = {}
LibDirs["shaderc"] = "C:/VulkanSDK/1.1.130.0/lib"
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
	}

    sysincludedirs 
    {
        "%{IncludeDir.glm}",
	}

    libdirs
    {
        "%{LibDirs.shaderc}/Debug",
        "%{LibDirs.assimp}"
    }

    links 
	{ 
        "GLFW",
        "assimp",
        "shaderc_combined",
        "C:/VulkanSDK/1.1.130.0/Lib/vulkan-1.lib"
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

    filter "configurations:Debug_Vulkan"
        defines { "IS_DEBUG", "IS_VULKAN", "IS_PROFILE", "NOMINMAX" }
        symbols "on"

    filter "configurations:Debug_OpenGL"
        defines { "IS_DEBUG", "IS_OPENGL", "IS_PROFILE", "NOMINMAX" }
        symbols "on"

    filter "configurations:Debug_Vulkan_SP"
        defines { "IS_DEBUG", "IS_VULKAN", "IS_SMART_POINTERS_IN_USE", "IS_PROFILE" }
        symbols "on"

    filter "configurations:Debug_OpenGL_SP"
        defines { "IS_DEBUG", "IS_OPENGL", "IS_SMART_POINTERS_IN_USE", "IS_PROFILE" }
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
        "%{IncludeDir.spdlog}",
        "%{IncludeDir.assimp}",
        "%{IncludeDir.glm}",
        "%{IncludeDir.imgui}",
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


    filter "configurations:Debug_Vulkan"
        defines { "IS_DEBUG", "IS_VULKAN", "IS_PROFILE", "NOMINMAX" }
        symbols "on"

    filter "configurations:Debug_OpenGL"
        defines { "IS_DEBUG", "IS_OPENGL", "IS_PROFILE", "NOMINMAX" }
        symbols "on"   
        
    filter "configurations:Debug_Vulkan_SP"
        defines { "IS_DEBUG", "IS_VULKAN", "IS_SMART_POINTERS_IN_USE", "IS_PROFILE" }
        symbols "on"

    filter "configurations:Debug_OpenGL_SP"
        defines { "IS_DEBUG", "IS_OPENGL", "IS_SMART_POINTERS_IN_USE", "IS_PROFILE" }
        symbols "on"

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
        "%{IncludeDir.spdlog}",
        "%{IncludeDir.assimp}",
        "%{IncludeDir.glm}",
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

    filter "configurations:Debug_Vulkan"
        defines { "IS_DEBUG", "IS_VULKAN" }
        symbols "on"

    filter "configurations:Debug_OpenGL"
        defines { "IS_DEBUG", "IS_OPENGL" }
        symbols "on"

    filter "configurations:Debug_Vulkan_SP"
        defines { "IS_DEBUG", "IS_VULKAN", "IS_SMART_POINTERS_IN_USE" }
        symbols "on"

    filter "configurations:Debug_OpenGL_SP"
        defines { "IS_DEBUG", "IS_OPENGL", "IS_SMART_POINTERS_IN_USE" }
        symbols "on"