local CommonConfig = require "../../lua/CommonConfig"

local RuntimeConfig = { }

function RuntimeConfig.DefinesSharedLib()
    defines
    {
        "IS_EXPORT_RUNTIME_DLL",
        "IS_AUDIO_MINIAUDIO_ENABLE",
        "ANIMATION_NODE_TRANSFORMS=0",
    }
end

function RuntimeConfig.DefinesStaticLib()
    defines
    {
        "IS_AUDIO_MINIAUDIO_ENABLE",
        "ANIMATION_NODE_TRANSFORMS=0",
    }
end

function RuntimeConfig.IncludeDirs()
    includedirs
    {
        "inc",

        "%{IncludeDirs.spdlog}",
        "%{IncludeDirs.glm}",
        "%{IncludeDirs.imgui}",
        "%{IncludeDirs.glfw}",
        "%{IncludeDirs.stb_image}",
        "%{IncludeDirs.splash}",
        "%{IncludeDirs.qoi}",
        "%{IncludeDirs.zip}",
        "%{IncludeDirs.miniaudio}",
        
        "%{IncludeDirs.assimp}",
        "%{IncludeDirs.assimp}/../build/include",
        "%{IncludeDirs.meshoptimizer}",
        "%{IncludeDirs.simplygon}",
        "%{IncludeDirs.reflect}",
        "%{IncludeDirs.nvrhi}",
    }
end

function RuntimeConfig.LibraryDirs()
    libdirs
    {
        "%{wks.location}/deps/lib",
    }
end

function RuntimeConfig.LibraryLinks()
    links
    {  
        "glm",
        "imgui",
        "zip",
        "meshoptimizer",
    }
    if (profileTool == "pix") then
        links
        {
            "WinPixEventRuntime"
        }
    end
end

function RuntimeConfig.FilterConfigurations()
    CommonConfig.FilterConfigurations()

    filter "configurations:Debug or configurations:Testing"
    buildoptions "/MDd"
    defines
    {
        "_DEBUG",
        "IS_DEBUG",
    }
    links
    {
        "Splashd",
        "glm",
        "imgui",
        "xxHashd",
        "meshoptimizer",
        "lz4d",
        "Reflectd",
        "assimp-vc143-mtd",

        --"GLFW",
        --"vulkan-1",
        --"D3d12",
        --"DXGI",
        --"dxguid",
        --"dxcompiler",
        --"GFSDK_Aftermath_Lib.x64",

        --"GenericCodeGen",
        --"glslang",
        --"glslang-default-resource-limits",
        --"MachineIndependent",
        --"OSDependent",
        --"SPIRV",
        --"SPIRV-Tools",
        --"SPIRV-Tools-opt",
        --"SPVRemapper",

        --"ffx_fsr2_api_x64d",
        --"ffx_fsr2_api_vk_x64d",
        --"ffx_fsr2_api_dx12_x64d",
    }
    prebuildcommands { "{COPYDIR} \"%{wks.location}deps/" .. outputdir .. "/dll/\" \"%{cfg.targetdir}\"", "{COPYDIR} \"%{wks.location}deps/" .. outputdir .. "/pdb/\" \"%{cfg.targetdir}\"",  }

filter "configurations:Release"
buildoptions "/MD"
    optimize "On"   
            defines
    {
        "NDEBUG",
        "IS_RELEASE",
        "DOCTEST_CONFIG_DISABLE",
    }
    links
    {
        "Splash",
        "glm",
        "imgui",
        "xxHash",
        "meshoptimizer",
        "lz4",
        "Reflect",
        "assimp-vc143-mt",

        --"GLFW",
        --"vulkan-1",
        --"D3d12",
        --"DXGI",
        --"dxguid",
        --"dxcompiler",
        --"GFSDK_Aftermath_Lib.x64",

        --"glslang",
        --"MachineIndependent",
        --"GenericCodeGen",
        --"OGLCompiler",
        --"OSDependent",

        --"ffx_fsr2_api_x64",
        --"ffx_fsr2_api_vk_x64",
        --"ffx_fsr2_api_dx12_x64",
    }
    prebuildcommands { "{COPYDIR} \"%{wks.location}deps/" .. outputdir .. "/dll/\" \"%{cfg.targetdir}\"", "{COPYDIR} \"%{wks.location}deps/" .. outputdir .. "/pdb/\" \"%{cfg.targetdir}\"", }
    
    filter "configurations:Testing" 
    links
    {
        --"doctest",
    }
    libdirs
    {
        "%{LibDirs.deps_testing_lib}",
    }
end

function RuntimeConfig.FilterPlatforms(AMD_Ryzen_Master_SDK, OutputDir)
    filter "system:Windows"
    	system "windows"
        defines
        {
            "IS_PLATFORM_WINDOWS",
            "IS_PLATFORM_WIN32",
            "IS_MATHS_DIRECTX_MATHS",
            --"IS_MATHS_CONSTRUCTOR_GLM",
            "IS_DX12_ENABLED",
            "IS_VULKAN_ENABLED",
            "IS_CPP_WINRT",

            "NOMINMAX",

            "SPLASH_PLATFORM_WINDOWS",
            
            "VK_USE_PLATFORM_WIN32_KHR",
        }
        if VULKAN_SDK == nil then
        else
            defines
            {
                "IS_VULKAN_ENABLED",
            }
        end
        includedirs
         { 
            "%{IncludeDirs.pix}",
         }
        links
        {
            "WinPixEventRuntime",
            "cppwinrt_fast_forwarder",
            "Xinput",
            "Comctl32",
        }

        filter "system:Unix"
    	system "linux"
    	toolset("clang")
        defines
        {
            "IS_PLATFORM_LINUX",
            "IS_VULKAN_ENABLED",
        }
end

return RuntimeConfig