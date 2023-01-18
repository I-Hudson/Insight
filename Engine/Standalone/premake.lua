project "InsightStandalone"  
    configurations { "Debug", "Release" } 
    location "./"
    kind "ConsoleApp"

    targetname ("%{prj.name}" .. output_project_subfix)
    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
    debugdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")

    defines
    {
        "IS_MONOLITH",
        "_CRT_SECURE_NO_WARNINGS",
        "GLM_FORCE_SWIZZLE",
        "GLM_FORCE_LEFT_HANDED",
        "GLM_FORCE_DEPTH_ZERO_TO_ONE",

        "IS_PLATFORM_X64",
        "IS_MEMORY_TRACKING",
        "RENDER_GRAPH_ENABLED",
        "TOBJECTPTR_REF_COUNTING",

        "COMMAND_LIST_RENDER_BATCH",
        "IS_NVIDIA_AFTERMATH_ENABLED",
    }

    includedirs
    {
        "inc",
        "%{IncludeDirs.InsightCore}",
        "%{IncludeDirs.InsightMaths}",
        "%{IncludeDirs.InsightInput}",
        "%{IncludeDirs.InsightGraphics}",
        "%{IncludeDirs.InsightRuntime}",

        "%{IncludeDirs.spdlog}",
        "%{IncludeDirs.doctest}",
        "%{IncludeDirs.imgui}",
        "%{IncludeDirs.xxHash}",
        "%{IncludeDirs.glm}",
        "%{IncludeDirs.glfw}",
        "%{IncludeDirs.stb_image}",
        "%{IncludeDirs.splash}",

        "%{IncludeDirs.assimp}",
        "%{IncludeDirs.assimp}/../build/include",
        "%{IncludeDirs.meshoptimizer}",

        "%{IncludeDirs.vma}",
        "%{IncludeDirs.vulkan}",
        "%{IncludeDirs.spirv_reflect}",
        "%{IncludeDirs.FFR2}",
        "%{IncludeDirs.Aftermath}",
        "%{IncludeDirs.DirectXHeaders}",
        "%{IncludeDirs.DirectXAgilityHeaders}",
        "%{IncludeDirs.DirectXShaderCompiler}",
        "%{IncludeDirs.D3D12MemoryAllocator}",
    }

    files 
    { 
        "../../Engine/Core/inc/**.hpp", 
        "../../Engine/Core/inc/**.h", 
        "../../Engine/Core/src/**.cpp",

        "../../Engine/Maths/inc/**.hpp", 
        "../../Engine/Maths/inc/**.h", 
        "../../Engine/Maths/src/**.cpp", 

        "../../Engine/Input/inc/**.hpp", 
        "../../Engine/Input/inc/**.h", 
        "../../Engine/Input/src/**.cpp", 

        "../../Engine/Graphics//inc/**.hpp", 
        "../../Engine/Graphics/inc/**.h", 
        "../../Engine/Graphics/src/**.cpp", 

        "../../Engine/Runtime/inc/**.hpp", 
        "../../Engine/Runtime/inc/**.h", 
        "../../Engine/Runtime/src/**.cpp",

        "inc/**.hpp", 
        "inc/**.h", 
        "src/**.cpp",

        "../../vendor/stb/stb_image.h",
        "../../vendor/stb/stb_image_write.h",
        "../../vendor/SPIRV-Reflect/spirv_reflect.h",
        "../../vendor/SPIRV-Reflect/spirv_reflect.cpp",
    }

    filter "system:Windows"
        files
        {
            "../../vendor/D3D12MemoryAllocator/src/D3D12MemAlloc.cpp",
            "../../vendor/D3D12MemoryAllocator/include/D3D12MemAlloc.h",
        }

    links
    {
        "Insight_Core" .. output_project_subfix .. ".lib",
        "Insight_Maths" .. output_project_subfix .. ".lib",
        "Insight_Input" .. output_project_subfix .. ".lib",
        
        "GLFW.lib",
        "glm.lib",
        "imgui.lib",
        "vulkan-1.lib",
        "D3d12.lib",
        "DXGI.lib",
        "dxguid.lib",
        "dxcompiler.lib",
        "Optickcore.lib",
        "tracy.lib",
        "meshoptimizer.lib",
        "GFSDK_Aftermath_Lib.x64.lib",
    }
    
    libdirs
    {
        "%{LibDirs.vulkan}",
    }

    filter { "kind:SharedLib or SharedLib" }
        postbuildcommands
        {
            "{COPY} \"%{cfg.targetdir}/%{prj.name}" .. output_project_subfix .. ".dll\" \"%{wks.location}deps/".. outputdir..  "/dll/\"\n",
            "{COPY} \"%{cfg.targetdir}/%{prj.name}" .. output_project_subfix .. ".lib\" \"%{wks.location}deps/".. outputdir..  "/lib/\"\n",
            "{COPY} \"%{cfg.targetdir}/%{prj.name}" .. output_project_subfix .. ".pdb\" \"%{wks.location}deps/".. outputdir..  "/pdb/\"\n",
            "{COPY} \"%{cfg.targetdir}/%{prj.name}" .. output_project_subfix .. ".dll\" \"%{wks.location}bin/".. outputdir..  "/" .. output_executable .. "/\"\n",
        }
    filter { "kind:StaticLib or StaticLib" }
        postbuildcommands
        {
            "{COPY} \"%{cfg.targetdir}/%{prj.name}" .. output_project_subfix .. ".lib\" \"%{wks.location}deps/".. outputdir..  "/lib/\"\n",
            "{COPY} \"%{cfg.targetdir}/%{prj.name}" .. output_project_subfix .. ".pdb\" \"%{wks.location}deps/".. outputdir..  "/pdb/\"\n",
        }

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
            "OptickCore.lib",
            "tracy.lib",
            "glm.lib",
            "imgui.lib",
            "xxHashd.lib",
            "meshoptimizer.lib",

            "GLFW.lib",
            "vulkan-1.lib",
            "D3d12.lib",
            "DXGI.lib",
            "dxguid.lib",
            "dxcompiler.lib",
            "GFSDK_Aftermath_Lib.x64.lib",

            "GenericCodeGend.lib",
            "glslangd.lib",
            "glslang-default-resource-limitsd.lib",
            "HLSLd.lib",
            "MachineIndependentd.lib",
            "OGLCompilerd.lib",
            "OSDependentd.lib",
            "SPIRVd.lib",
            "SPIRV-Toolsd.lib",
            "SPIRV-Tools-optd.lib",
            "SPVRemapperd.lib",
            "assimpd.lib",

            "ffx_fsr2_api_x64d.lib",
            "ffx_fsr2_api_vk_x64d.lib",
            "ffx_fsr2_api_dx12_x64d.lib",
        }
        prebuildcommands { "{COPYDIR} \"%{wks.location}deps/" .. outputdir .. "/dll/\" \"%{cfg.targetdir}\"", "{COPYDIR} \"%{wks.location}deps/" .. outputdir .. "/pdb/\" \"%{cfg.targetdir}\"",  }

    filter "configurations:Release"  
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
            "OptickCore.lib",
            "tracy.lib",
            "glm.lib",
            "imgui.lib",
            "xxHashd.lib",
            "meshoptimizer.lib",

            "GLFW.lib",
            "vulkan-1.lib",
            "D3d12.lib",
            "DXGI.lib",
            "dxguid.lib",
            "dxcompiler.lib",
            "GFSDK_Aftermath_Lib.x64.lib",

            "glslang.lib",
            "MachineIndependent.lib",
            "GenericCodeGen.lib",
            "OGLCompiler.lib",
            "OSDependent.lib",
            "Optickcore.lib",
            "tracy.lib",
            "assimp.lib",

            "ffx_fsr2_api_x64.lib",
            "ffx_fsr2_api_vk_x64.lib",
            "ffx_fsr2_api_dx12_x64.lib",
        }
        prebuildcommands { "{COPYDIR} \"%{wks.location}deps/" .. outputdir .. "/dll/\" \"%{cfg.targetdir}\"", "{COPYDIR} \"%{wks.location}deps/" .. outputdir .. "/pdb/\" \"%{cfg.targetdir}\"", }
        
    filter "configurations:Testing" 
        links
        {
            "doctest.lib",
        }
        libdirs
        {
            "%{LibDirs.deps_testing_lib}",
        }

    filter "system:Windows"
    	system "windows"
    	toolset("msc-v142")
        defines
        {
            "IS_PLATFORM_WINDOWS",
            "IS_PLATFORM_WIN32",
            "IS_MATHS_DIRECTX_MATHS",
            "IS_DX12_ENABLED",
            "IS_VULKAN_ENABLED",
            "IS_CPP_WINRT",

            "NOMINMAX",

            "SPLASH_PLATFORM_WINDOWS",
            
            "VK_USE_PLATFORM_WIN32_KHR",
        }
        includedirs
         { 
            "%{IncludeDirs.pix}",
         }
        links
        {
            "WinPixEventRuntime.lib",
            "cppwinrt_fast_forwarder.lib",
            "Xinput.lib",
        }
        files
        {
            "../../vendor/D3D12MemoryAllocator/src/D3D12MemAlloc.cpp",
            "../../vendor/D3D12MemoryAllocator/include/D3D12MemAlloc.h",
        }

    filter { "system:Windows", "configurations:Debug" or "configurations:Testing" }
        ignoredefaultlibraries
        {
            "libcmt.lib",
            "msvcrt.lib",
            "libcmtd.lib",
        }
    filter { "system:Windows", "configurations:Release" }
        ignoredefaultlibraries
        {
            "libcmt.lib",
            "libcmtd.lib",
            "msvcrtd.lib",
        }

    filter "configurations:Testing"
        defines
        {
            --"TESTING",
            "TEST_ENABLED",
            --"DOCTEST_CONFIG_IMPLEMENTATION_IN_DLL",
        }
        files 
        { 
            "vendor/doctest/doctest/doctest.h",
        } 
        libdirs
        {
            "%{LibDirs.deps_testing_lib}",
        }
    	
    
    filter "system:Unix"
    	system "linux"
    	toolset("clang")
        defines
        {
            "IS_PLATFORM_LINUX",
            "IS_VULKAN_ENABLED",
        }