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
        "%{IncludeDirs.InsightCore}",
        "%{IncludeDirs.InsightMaths}",
        "%{IncludeDirs.InsightPhysics}",
        "%{IncludeDirs.InsightGraphics}",
        "%{IncludeDirs.InsightInput}",

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
        "Insight_Core" .. output_project_subfix .. ".lib",
        "Insight_Maths" .. output_project_subfix .. ".lib",
        "Insight_Physics" .. output_project_subfix .. ".lib",
        "Insight_Graphics" .. output_project_subfix .. ".lib",
        "Insight_Input" .. output_project_subfix .. ".lib",
        
        "glm.lib",
        "imgui.lib",
        "zip.lib",
        "meshoptimizer.lib",
    }
    if (profileTool == "pix") then
        links
        {
            "WinPixEventRuntime.lib"
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
        "Splashd.lib",
        "glm.lib",
        "imgui.lib",
        "xxHashd.lib",
        "meshoptimizer.lib",
        "lz4d.lib",
        "Reflectd.lib",
        "assimpd.lib",

        --"GLFW.lib",
        --"vulkan-1.lib",
        --"D3d12.lib",
        --"DXGI.lib",
        --"dxguid.lib",
        --"dxcompiler.lib",
        --"GFSDK_Aftermath_Lib.x64.lib",

        --"GenericCodeGen.lib",
        --"glslang.lib",
        --"glslang-default-resource-limits.lib",
        --"MachineIndependent.lib",
        --"OSDependent.lib",
        --"SPIRV.lib",
        --"SPIRV-Tools.lib",
        --"SPIRV-Tools-opt.lib",
        --"SPVRemapper.lib",

        --"ffx_fsr2_api_x64d.lib",
        --"ffx_fsr2_api_vk_x64d.lib",
        --"ffx_fsr2_api_dx12_x64d.lib",
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
        "Splash.lib",
        "glm.lib",
        "imgui.lib",
        "xxHash.lib",
        "meshoptimizer.lib",
        "lz4.lib",
        "Reflect.lib",
        "assimp.lib",

        --"GLFW.lib",
        --"vulkan-1.lib",
        --"D3d12.lib",
        --"DXGI.lib",
        --"dxguid.lib",
        --"dxcompiler.lib",
        --"GFSDK_Aftermath_Lib.x64.lib",

        --"glslang.lib",
        --"MachineIndependent.lib",
        --"GenericCodeGen.lib",
        --"OGLCompiler.lib",
        --"OSDependent.lib",

        --"ffx_fsr2_api_x64.lib",
        --"ffx_fsr2_api_vk_x64.lib",
        --"ffx_fsr2_api_dx12_x64.lib",
    }
    prebuildcommands { "{COPYDIR} \"%{wks.location}deps/" .. outputdir .. "/dll/\" \"%{cfg.targetdir}\"", "{COPYDIR} \"%{wks.location}deps/" .. outputdir .. "/pdb/\" \"%{cfg.targetdir}\"", }
    
    filter "configurations:Testing" 
    links
    {
        --"doctest.lib",
    }
    libdirs
    {
        "%{LibDirs.deps_testing_lib}",
    }
end

function RuntimeConfig.FilterPlatforms(AMD_Ryzen_Master_SDK, OutputDir)
    filter "system:Windows"
    	system "windows"
    	toolset("msc-v143")
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
            "WinPixEventRuntime.lib",
            "cppwinrt_fast_forwarder.lib",
            "Xinput.lib",
            "Comctl32.lib",
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