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
IncludeDir["Vulkan"] = os.getenv("VK_SDK_PATH") .. "/Include"
IncludeDir["glm"] = "$(SolutionDir)Insight/vendor/glm"
IncludeDir["stb_image"] = "$(SolutionDir)Insight/vendor/stb_image"
IncludeDir["glad"] = "$(SolutionDir)Insight/vendor/glad/include"
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
        "%{IncludeDir.optick}",
        "%{IncludeDir.tinygltf}",
        "%{IncludeDir.ktx}",
	}

    sysincludedirs 
    {
        "%{IncludeDir.glm}",
	}

    libdirs
    {
        "%{LibDirs.Vulkan}",
        "%{LibDirs.optick}",
        "%{LibDirs.ktx}",
	}

    links 
	{ 
        "GLFW",
        "OptickCore",
        "ktx",
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
        defines { "IS_DEBUG", "IS_OPENGL", "IS_PROFILE", "NOMINMAX", "IS_EDITOR", "IMGUI_ENABLED", }
        symbols "on"
        links 
        {
            "assimp-vc142-mtd",
            "opengl32"
	    }
        libdirs
        {
            "%{LibDirs.assimp}/Debug"
        }

    filter "configurations:Debug"
        defines { "IS_DEBUG", "IS_VULKAN", "NOMINMAX", "IS_EDITOR", "GLFW_INCLUDE_VULKAN", "IMGUI_ENABLED", "IS_OPTICK_PROFILE" }
        symbols "on"
        links 
        {
            "assimp-vc142-mtd",
            "shaderc_combined",
            "vulkan-1",
	    }
        libdirs
        {
            "%{LibDirs.Vulkan}/Debug",
            "%{LibDirs.assimp}/Debug"
        }

    filter "configurations:Debug_NO_EDITOR"
        defines { "IS_DEBUG", "IS_VULKAN", "NOMINMAX", "GLFW_INCLUDE_VULKAN", "IS_OPTICK_PROFILE" }
        symbols "on"
        links 
        {
            "assimp-vc142-mtd",
            "shaderc_combined",
            "vulkan-1",
	    }
        libdirs
        {
            "%{LibDirs.Vulkan}/Debug",
            "%{LibDirs.assimp}/Debug"
        }

    filter "configurations:Release"
        defines { "IS_RELEASE", "IS_VULKAN", "NOMINMAX", "IS_EDITOR", "GLFW_INCLUDE_VULKAN", "IMGUI_ENABLED", "IS_OPTICK_PROFILE" }
        optimize "on"
        links 
        {
            "assimp-vc142-mt",
            "shaderc_combined",
            "vulkan-1",
	    }
        libdirs
        {
            "%{LibDirs.Vulkan}/Release",
            "%{LibDirs.assimp}/Release"
        }

    filter "configurations:Release_NO_EDITOR"
        defines { "IS_RELEASE", "IS_VULKAN", "NOMINMAX", "GLFW_INCLUDE_VULKAN", "IS_OPTICK_PROFILE" }
        optimize "on"
        links 
        {
            "assimp-vc142-mt",
            "shaderc_combined",
            "vulkan-1",
	    }
        libdirs
        {
            "%{LibDirs.Vulkan}/Release",
            "%{LibDirs.assimp}/Release"
        }

    filter "configurations:Dist"
        defines { "IS_DIST", "IS_VULKAN", "NOMINMAX", "GLFW_INCLUDE_VULKAN", }
        optimize "full"
        links 
        {
            "assimp-vc142-mt",
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
        "%{IncludeDir.optick}",
        "%{IncludeDir.Vulkan}",

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
        defines { "IS_DEBUG", "IS_VULKAN", "NOMINMAX", "IS_EDITOR", "GLFW_INCLUDE_VULKAN" }
        symbols "on"
        kind "ConsoleApp"

    filter "configurations:Debug_NO_VULKAN"
        defines { "IS_DEBUG", "IS_OPENGL", "NOMINMAX", "IS_EDITOR" }
        symbols "on"
        kind "ConsoleApp"

    filter "configurations:Debug_NO_EDITOR"
        defines { "IS_DEBUG", "IS_VULKAN", "NOMINMAX", "GLFW_INCLUDE_VULKAN" }
        symbols "on"
        kind "ConsoleApp"

    filter "configurations:Release"
        defines { "IS_RELEASE", "IS_VULKAN", "NOMINMAX", "IS_EDITOR", "GLFW_INCLUDE_VULKAN" }
        optimize "on"
        kind "WindowedApp"

    filter "configurations:Release_NO_EDITOR"
        defines { "IS_RELEASE", "IS_VULKAN", "NOMINMAX", "GLFW_INCLUDE_VULKAN" }
        optimize "on"
        kind "WindowedApp"

    filter "configurations:Dist"
        defines { "IS_DIST", "IS_VULKAN", "NOMINMAX", "GLFW_INCLUDE_VULKAN" }
        optimize "full"
        kind "WindowedApp"

        filter { "system:windows", "configurations:Release" }
            buildoptions "/MT"