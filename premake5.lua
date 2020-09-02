workspace "Insight"
    architecture "x64"
    startproject "Sandbox"

    configurations
    {
        "Debug",
        "Debug_NO_VULKAN",
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
IncludeDir["assimp_gen"] = "$(SolutionDir)Insight/vendor/assimp/BINARIES/x64/Include"
IncludeDir["Vulkan"] = "C:/VulkanSDK/1.1.130.0/Include"
IncludeDir["glm"] = "$(SolutionDir)Insight/vendor/glm"
IncludeDir["stb_image"] = "$(SolutionDir)Insight/vendor/stb_image"
IncludeDir["glad"] = "$(SolutionDir)Insight/vendor/glad/include"
IncludeDir["imgui"] = "$(SolutionDir)Insight/vendor/imgui/"
IncludeDir["TinyXML2"] = "$(SolutionDir)Insight/vendor/TinyXML2"

-- Lib directories relative to root folder (solution directory)
LibDirs = {}
LibDirs["Vulkan"] = "C:/VulkanSDK/1.1.130.0/lib"
LibDirs["assimp"] = "$(ProjectDir)vendor/assimp/BINARIES/x64/lib"

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
        "%{prj.name}/vendor/TinyXML2/**.h",
		"%{prj.name}/vendor/TinyXML2/**.cpp",
	}

    includedirs 
    {
		"$(ProjectDir)src",
		"%{IncludeDir.spdlog}",
        "%{IncludeDir.GLFW}",
        "%{IncludeDir.assimp}",
        "%{IncludeDir.assimp_gen}",
        "%{IncludeDir.Vulkan}",
        "%{IncludeDir.glm}",
        "%{IncludeDir.stb_image}",
        "%{IncludeDir.glad}",
        "%{IncludeDir.imgui}",
        "%{IncludeDir.TinyXML2}",
	}

    sysincludedirs 
    {
        "%{IncludeDir.glm}",
	}

    libdirs
    {
        "%{LibDirs.Vulkan}"
	}

    links 
	{ 
        "GLFW"
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

    filter "configurations:Debug_NO_VULKAN"
        defines { "IS_DEBUG", "IS_OPENGL", "IS_PROFILE", "NOMINMAX", "IS_EDITOR" }--, "IMGUI_ENABLED" }
        symbols "on"
        links 
        {
            "assimp-vc141-mtd",
            "opengl32"
	    }
        libdirs
        {
            "%{LibDirs.assimp}/Debug"
        }

    filter "configurations:Debug"
        defines { "IS_DEBUG", "IS_VULKAN", "IS_PROFILE", "NOMINMAX", "IS_EDITOR", "IMGUI_ENABLED", "GLFW_INCLUDE_VULKAN" }
        symbols "on"
        links 
        {
            "assimp-vc141-mtd",
            "shaderc_combined",
            "vulkan-1",
	    }
        libdirs
        {
            "%{LibDirs.Vulkan}/Debug",
            "%{LibDirs.assimp}/Debug"
        }

    filter "configurations:Debug_NO_EDITOR"
        defines { "IS_DEBUG", "IS_VULKAN", "IS_PROFILE", "NOMINMAX", "GLFW_INCLUDE_VULKAN" }
        symbols "on"
        links 
        {
            "assimp-vc141-mtd",
            "shaderc_combined",
            "vulkan-1",
	    }
        libdirs
        {
            "%{LibDirs.Vulkan}/Debug",
            "%{LibDirs.assimp}/Debug"
        }

    filter "configurations:Release"
        defines { "IS_RELEASE", "IS_VULKAN", "IS_PROFILE", "NOMINMAX", "IS_EDITOR", "IMGUI_ENABLED", "GLFW_INCLUDE_VULKAN" }
        optimize "on"
        links 
        {
            "assimp-vc141-mt",
            "shaderc_combined",
            "vulkan-1",
	    }
        libdirs
        {
            "%{LibDirs.Vulkan}/Release",
            "%{LibDirs.assimp}/Release"
        }

    filter "configurations:Release_NO_EDITOR"
        defines { "IS_RELEASE", "IS_VULKAN", "IS_PROFILE", "NOMINMAX", "GLFW_INCLUDE_VULKAN" }
        optimize "on"
        links 
        {
            "assimp-vc141-mt",
            "shaderc_combined",
            "vulkan-1",
	    }
        libdirs
        {
            "%{LibDirs.Vulkan}/Release",
            "%{LibDirs.assimp}/Release"
        }

    filter "configurations:Dist"
        defines { "IS_DIST", "IS_VULKAN", "IS_PROFILE", "NOMINMAX", "GLFW_INCLUDE_VULKAN" }
        optimize "full"
        links 
        {
            "assimp-vc141-mt",
            "shaderc_combined",
            "vulkan-1",
	    }
        libdirs
        {
            "%{LibDirs.Vulkan}/Release",
            "%{LibDirs.assimp}/Release"
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
        "%{IncludeDir.assimp_gen}",
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


    filter "configurations:Debug"
        defines { "IS_DEBUG", "IS_VULKAN", "IS_PROFILE", "NOMINMAX", "IS_EDITOR", "IMGUI_ENABLED", "GLFW_INCLUDE_VULKAN" }
        symbols "on"
        kind "ConsoleApp"

    filter "configurations:Debug_NO_VULKAN"
        defines { "IS_DEBUG", "IS_OPENGL", "IS_PROFILE", "NOMINMAX", "IS_EDITOR" }--, "IMGUI_ENABLED" }
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
        "%{IncludeDir.assimp_gen}",
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

    filter "configurations:Debug_NO_VULKAN"
        defines { "IS_DEBUG", "IS_OPENGL", "IS_PROFILE", "NOMINMAX", "IS_EDITOR", "IMGUI_ENABLED" }
        symbols "on"