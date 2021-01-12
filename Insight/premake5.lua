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
		"src/**.hpp",
        "src/**.cpp",
        "vendor/stb_image/**.h",
		"vendor/stb_image/**.cpp",
        "vendor/glm/glm/**.hpp",
		"vendor/glm/glm/**.inl",
        "vendor/TinyXML2/**.h",
		"vendor/TinyXML2/**.cpp",
		"vendor/IconFontCppHeaders/**.h",
	}

    includedirs 
    {
		"$(ProjectDir)src",
		"$(ProjectDir)src/Insight",
        "%{IncludeDir.GLFW}",
        "%{IncludeDir.Glad}",
        "%{IncludeDir.imgui}",
		"%{IncludeDir.spdlog}",
        "%{IncludeDir.assimp}",
        "%{IncludeDir.assimp_gen}",
        "%{IncludeDir.Vulkan}",
        "%{IncludeDir.glm}",
        "%{IncludeDir.stb_image}",
        "%{IncludeDir.TinyXML2}",
        "%{IncludeDir.json}",
        "%{IncludeDir.optick}",
        "%{IncludeDir.tinygltf}",
        "%{IncludeDir.ktx}",
        "%{IncludeDir.IconsFontAwesome}",
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
        "Glad",
        "ImGui",

        "OptickCore",
        "ktx",
        "shaderc_combined",
        "vulkan-1",
	}

    filter "system:windows"
        systemversion "latest"

        defines 
        {  
            "IS_PLATFORM_WINDOWS",
            "GLM_FORCE_SWIZZLE",
            "GLM_FORCE_SILENT_WARNINGS",
            "IS_STANDARD_POINTER",
        }

    filter "configurations:Debug"
        defines { "IS_DEBUG", "IS_VULKAN", "NOMINMAX", "IS_EDITOR", "GLFW_INCLUDE_VULKAN", "IMGUI_ENABLED" }
        symbols "on"
        links 
        {
            "assimp-vc142-mtd",
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
        }
        
        libdirs
        {
            "%{LibDirs.Vulkan}/Release",
            "%{LibDirs.assimp}/Release"
        }

    filter { "system:windows", "configurations:Release" }
        buildoptions "/MT"