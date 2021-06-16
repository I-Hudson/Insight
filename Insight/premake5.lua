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
		"inc/**.h",
		"inc/**.cpp",
		"inc/**.hpp",
        "src/**.cpp",
        "vendor/glm/glm/**.hpp",
		"vendor/glm/glm/**.inl",
        "vendor/TinyXML2/**.h",
		"vendor/TinyXML2/**.cpp",
        "vendor/stb/stb_image.h",
        "vendor/stb/stb_image_write.h",
		"vendor/IconFontCppHeaders/**.h",
		"vendor/SPIRV-Reflect/spirv_reflect.h",
		"vendor/SPIRV-Reflect/spirv_reflect.cpp",
		"vendor/tinygltf/tiny_gltf.h",
		"vendor/tinygltf/json.hpp",
		"vendor/tinygltf/stb_image.h",
		"vendor/tinygltf/stb_image_write.h",
	}

    includedirs 
    {
		"$(ProjectDir)inc",
        "$(SolutionDir)InsightEditor/inc",
		"$(ProjectDir)inc/Insight",
		"$(ProjectDir)inc/Platform",
		"$(ProjectDir)inc/Platform/Vulkan",
        "%{IncludeDir.GLFW}",
        "%{IncludeDir.Glad}",
        "%{IncludeDir.imguizmo}",
        "%{IncludeDir.imgui}",
		"%{IncludeDir.spdlog}",
        "%{IncludeDir.assimp}",
        "%{IncludeDir.assimp_gen}",
        "%{IncludeDir.glm}",
        "%{IncludeDir.stb_image}",
        "%{IncludeDir.TinyXML2}",
        "%{IncludeDir.json}",
        "%{IncludeDir.optick}",
        "%{IncludeDir.tinygltf}",
        "%{IncludeDir.IconsFontAwesome}",
        "%{IncludeDir.Reflect}",
        "%{IncludeDir.JobSystem}",

        "%{IncludeDir.glslang}",
        "%{IncludeDir.SPIRV_Reflect}",
        "%{IncludeDir.VulkanMemoryAllocator}",
        "%{IncludeDir.Vulkan}",
	}

    libdirs
    {
        "%{LibDirs.optick}",

        "%{LibDirs.glslang}/glslang/$(Configuration)",
        "%{LibDirs.glslang}/glslang/OSDependent/Windows/$(Configuration)",
        "%{LibDirs.glslang}/OGLCompilersDLL/$(Configuration)",
        "%{LibDirs.glslang}/SPIRV/$(Configuration)",
        "%{LibDirs.glslang}/StandAlone/$(Configuration)",
        "%{LibDirs.glslang}/OGLCompilerd/$(Configuration)",
        "%{LibDirs.Vulkan}",
	}

    links 
	{ 
        "GLFW",
        "Glad",
        "ImGui",
        "ImGuizmo",
        "JobSystem",
        "ReflectEXE",

        "OptickCore",
        "vulkan-1",
	}

    prebuildcommands
    {
        "\"$(ProjectDir)vendor/Reflect/bin/" .. outputdir .. "/ReflectEXE/ReflectEXE.exe\" \"$(ProjectDir)inc\" pchInclude=ispch.h",
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
            "IS_Y_IS_DOWN",
        }

    filter "configurations:Debug"
        defines { "IS_DEBUG", "IS_VULKAN", "NOMINMAX", "IS_EDITOR", "GLFW_INCLUDE_VULKAN", "IMGUI_ENABLED", "VMA_RECORDING_ENABLED" }
        symbols "on"
        links 
        {
            "assimp-vc142-mtd",
            "glslangd",
            "MachineIndependentd",
            "GenericCodeGend",
            "OSDependentd",
            "SPIRVd",
            "OGLCompilerd",
            "glslang-default-resource-limitsd",
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
            "glslang",
            "MachineIndependent",
            "GenericCodeGen",
            "OSDependent",
            "SPIRV",
            "OGLCompiler",
            "glslang-default-resource-limits",
        }
        
        libdirs
        {
            "%{LibDirs.Vulkan}/Release",
            "%{LibDirs.assimp}/Release"
        }

    filter "configurations:Dist"
        defines { "IS_DIST", "IS_VULKAN", "NOMINMAX",  "IS_EDITOR", "GLFW_INCLUDE_VULKAN", "IMGUI_ENABLED" }
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