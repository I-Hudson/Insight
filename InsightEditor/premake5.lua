project "InsightEditor"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
	staticruntime "on"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
    debugdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")


    files
	{
		"src/**.h",
        "src/**.cpp",
        "vendor/ImGui-Addons/FileBrowser/ImGuiFileBrowser.cpp",
        "vendor/ImGui-Addons/FileBrowser/ImGuiFileBrowser.h",
	}

    includedirs 
    {
		"$(ProjectDir)src",
        "$(SolutionDir)Insight/src",
        "%{IncludeDir.spdlog}",
        "%{IncludeDir.assimp}",
        "%{IncludeDir.assimp_gen}",
        "%{IncludeDir.glm}",
        "%{IncludeDir.imgui}",
        "%{IncludeDir.optick}",
        "%{IncludeDir.ktx}",
        "%{IncludeDir.Vulkan}",
        "%{IncludeDir.VulkanMemoryAllocator}",

        "%{IncludeDir.imguiFileDialog}",
        "%{IncludeDir.dirent}",
        "%{IncludeDir.IconsFontAwesome}",
	}

    links 
	{ 
        "Insight",
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
            "IMGUI_ENABLED",
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