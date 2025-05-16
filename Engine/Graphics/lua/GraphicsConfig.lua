local CommonConfig = require "../../lua/CommonConfig"

local GraphicsConfig = { }

local NvidiaAftermath_SDKPath = "../../vendor/NVIDIA_Nsight_Aftermath_SDK"

function GraphicsConfig.DefinesSharedLib()
    defines
    {
        "IS_EXPORT_GRAPHICS_DLL",
        "COMMAND_LIST_RENDER_BATCH",
    }

    if NvidiaAftermath_SDKEnabled then
        defines
        {
            "IS_NVIDIA_AFTERMATH_ENABLED",
        }
    end
end

function GraphicsConfig.DefinesStaticLib()
    defines
    {
        "COMMAND_LIST_RENDER_BATCH",
    }

    if NvidiaAftermath_SDKEnabled then
        defines
        {
            "IS_NVIDIA_AFTERMATH_ENABLED",
        }
    end
end

function GraphicsConfig.IncludeDirs()
    includedirs
    {
        "inc",
        "inc/FSR2",
        "inc/FSR2/DX12",

        "%{IncludeDirs.InsightCore}",
        "%{IncludeDirs.InsightMaths}",
        "%{IncludeDirs.InsightInput}",

        "%{IncludeDirs.glfw}",
        "%{IncludeDirs.glm}",
        "%{IncludeDirs.imgui}",
        "%{IncludeDirs.reflect}",
        "%{IncludeDirs.zip}",
        "%{IncludeDirs.spdlog}",
        "%{IncludeDirs.vma}",
        "%{IncludeDirs.glslang}win_debug/inc",
        "%{IncludeDirs.vulkan}",
        "%{IncludeDirs.spirv_reflect}",
        "%{IncludeDirs.assimp}",
        "%{IncludeDirs.assimp}/../build/include",
        "%{IncludeDirs.stb_image}",
        "%{IncludeDirs.meshoptimizer}",
        "%{IncludeDirs.FFR2}",
        "%{IncludeDirs.Aftermath}",
        "%{IncludeDirs.nvtx}",
        "%{IncludeDirs.DirectXHeaders}",
        "%{IncludeDirs.DirectXAgilityHeaders}",
        "%{IncludeDirs.DirectXShaderCompiler}",
        "%{IncludeDirs.D3D12MemoryAllocator}",
        "%{IncludeDirs.nvrhi}",
    }
end

function GraphicsConfig.LibraryDirs()
    libdirs
    {
        "%{LibDirs.vulkan}",
    }
end

function GraphicsConfig.LibraryLinks()
    links
    {
        "Insight_Core" .. output_project_subfix .. ".lib",
        "Insight_Maths" .. output_project_subfix .. ".lib",
        "Insight_Input" .. output_project_subfix .. ".lib",
        
        "GLFW.lib",
        "glm.lib",
        "imgui.lib",
        "zip.lib",

        "D3d12.lib",
        "DXGI.lib",
        "dxguid.lib",
        "dxcompiler.lib",
        "meshoptimizer.lib",
        "nvrhi.lib",
    }

    if VULKAN_SDK == nil then
    else
        links
        {
            "vulkan-1.lib",
        }
    end

    if os.rename(NvidiaAftermath_SDKPath, NvidiaAftermath_SDKPath)  == true then
        links
        {
            "GFSDK_Aftermath_Lib.x64.lib",
        }
    end
end

function GraphicsConfig.FilterConfigurations()
    CommonConfig.FilterConfigurations()
    
    filter "configurations:Debug or configurations:Testing"
        links
        {
            "GenericCodeGen.lib",
            "glslang.lib",
            "glslang-default-resource-limits.lib",
            "MachineIndependent.lib",
            "OSDependent.lib",
            "SPIRV.lib",
            "SPIRV-Tools.lib",
            "SPIRV-Tools-opt.lib",
            "SPVRemapper.lib",

            "ffx_fsr2_api_x64d.lib",
            "ffx_fsr2_api_dx12_x64d.lib",
        }
    if VULKAN_SDK == nil then
    else
        links
        {
            "ffx_fsr2_api_vk_x64d.lib",
        }
    end

filter "configurations:Release"  
    links
    {
        "glslang.lib",
        "MachineIndependent.lib",
        "GenericCodeGen.lib",
        "OSDependent.lib",
        "tracy.lib",
        "assimp.lib",

        "ffx_fsr2_api_x64.lib",
        "ffx_fsr2_api_dx12_x64.lib",
    }
    if VULKAN_SDK == nil then
    else
        links
        {
            "ffx_fsr2_api_vk_x64.lib",
        }
    end
end

function GraphicsConfig.FilterPlatforms(AMD_Ryzen_Master_SDK, OutputDir)
    filter "system:Windows"
        files
        {
            "../../vendor/D3D12MemoryAllocator/src/D3D12MemAlloc.cpp",
            "../../vendor/D3D12MemoryAllocator/include/D3D12MemAlloc.h",
        }

    filter "platforms:Win64"
        local renderDocPath = "C:/Program Files/RenderDoc"
        local renderDocExists = CommonConfig.PathExists(renderDocPath)
            print(renderDocExists)
        if renderDocExists then
            print("RenderDoc")
            defines
            {
                "RENDER_DOC_API",
            }
            includedirs
            {
                renderDocPath,
            }
            prebuildcommands
            {
                "{COPY} \"" .. renderDocPath .. "/renderdoc.dll\" \"%{wks.location}deps/".. outputdir..  "/dll/\"\n",
            }
        end
end

return GraphicsConfig