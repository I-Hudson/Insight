project "Sandbox"
    language "C++"
    cppdialect "C++17"
	staticruntime "on"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
    debugdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")

    files
	{
		"inc/**.h",
		"inc/**.hpp",
		"src/**.cpp"
	}

    includedirs
    {
        "$(ProjectDir)/inc",
        "$(SolutionDir)Insight/inc",
        "%{IncludeDir.spdlog}",
        "%{IncludeDir.assimp}",
        "%{IncludeDir.assimp_gen}",
        "%{IncludeDir.glm}",
        "%{IncludeDir.imgui}",
        "%{IncludeDir.optick}",
        "%{IncludeDir.Vulkan}",
        "%{IncludeDir.Reflect}",

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
            "IS_PLATFORM_WINDOWS",
            "IS_PLATFORM_WIN32",
            "GLM_FORCE_SWIZZLE",
            "GLM_FORCE_SILENT_WARNINGS",
            "IS_STANDARD_POINTER",
        }


    filter "configurations:Debug"
        defines { "IS_DEBUG", "IS_VULKAN", "NOMINMAX", "IS_EDITOR", "GLFW_INCLUDE_VULKAN" }
        symbols "on"
        kind "ConsoleApp"


    filter "configurations:Release"
        defines { "IS_RELEASE", "IS_VULKAN", "NOMINMAX", "IS_EDITOR", "GLFW_INCLUDE_VULKAN" }
        optimize "on"
        kind "WindowedApp"

    filter "configurations:Dist"
        defines { "IS_DIST", "IS_VULKAN", "NOMINMAX", "GLFW_INCLUDE_VULKAN" }
        optimize "full"
        kind "WindowedApp"

        filter { "system:windows", "configurations:Release" }
            buildoptions "/MT"