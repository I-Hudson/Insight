project "InsightGraphics"
    kind "SharedLib"
    language "C++"
    cppdialect "C++17"
	staticruntime "on"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
    debugdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")

    files
	{
		"inc/**.h",
		"inc/**.cpp",
		"inc/**.hpp",
        "src/**.cpp",
        "vendor/stb/stb_image.h",
        "vendor/stb/stb_image_write.h",
		"vendor/SPIRV-Reflect/spirv_reflect.h",
		"vendor/SPIRV-Reflect/spirv_reflect.c",
        "vendor/volk-1.2.170/volk.h",
	}

    includedirs 
    {
		"$(ProjectDir)inc",
		"$(ProjectDir)../InsightCore/inc",
		"$(ProjectDir)vendor/glad/include",
		"$(ProjectDir)vendor/GLFW/include",
		"$(ProjectDir)vendor/volk-1.2.170",
		"$(ProjectDir)vendor/glslang",

		"$(ProjectDir)../InsightCore/vendor/glm",
		"$(ProjectDir)../InsightCore/vendor/spdlog/include",

		"$(ProjectDir)vendor/stb",
		"$(ProjectDir)vendor/SPIRV-Reflect",
		"$(ProjectDir)vendor/VulkanMemoryAllocator/src",
        os.getenv("VK_SDK_PATH") .. "/Include",
	}

    libdirs
    {
        "$(ProjectDir)vendor/glad/bin/" .. outputdir,
        os.getenv("VK_SDK_PATH") .. "/lib/"
	}

    links 
	{
        "InsightCore",
        "glad",
        "GLFW",
        "GenericCodeGen",
        "glslang",
        "OGLCompilersDLL",
        "OSDependent",
        "SPIRV",
        "StandAlone",
	}

    filter "system:windows"
        systemversion "latest"

        defines 
        {  
            "INSIGHT_NAMESPACE=",

            "IS_PLATFORM_WINDOWS",
            "IS_PLATFORM_WIN32",
            "WIN32_LEAN_AND_MEAN",
            
            "GLM_FORCE_SWIZZLE",
            "GLM_FORCE_SILENT_WARNINGS",
            "GLM_FORCE_DEPTH_ZERO_TO_ONE",

            "IS_STANDARD_POINTER",
        }

    filter "configurations:Debug"
        defines { "IS_DEBUG", "IS_VULKAN", "NOMINMAX", "IS_EDITOR", "GLFW_INCLUDE_VULKAN", "IMGUI_ENABLED", "VMA_RECORDING_ENABLED" }
        symbols "on"

    filter "configurations:Release"
        defines { "IS_RELEASE", "IS_VULKAN", "NOMINMAX", "IS_EDITOR", "GLFW_INCLUDE_VULKAN", "IMGUI_ENABLED", "IS_OPTICK_PROFILE" }
        optimize "on"

    filter "configurations:Dist"
        defines { "IS_DIST", "IS_VULKAN", "NOMINMAX",  "IS_EDITOR", "GLFW_INCLUDE_VULKAN", "IMGUI_ENABLED" }
        optimize "full"

    filter { "system:windows", "configurations:Release" }
        buildoptions "/MT"