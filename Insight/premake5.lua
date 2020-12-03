project "Insight"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
	staticruntime "on"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
    debugdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")

    pchheader "ispch.h"
    pchsource "src/ispch.cpp"

    files
	{
		"src/**.h",
        "src/**.cpp",
        "vendor/stb_image/**.h",
		"vendor/stb_image/**.cpp",
        "vendor/glm/glm/**.hpp",
		"vendor/glm/glm/**.inl",
		"vendor/glad/src/**.cpp",
		"vendor/imgui/**.h",
		"vendor/imgui/**.cpp",
        "vendor/TinyXML2/**.h",
		"vendor/TinyXML2/**.cpp",
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