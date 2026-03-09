local CommonConfig = require "../../lua/CommonConfig"

local GraphicsConfig = { }

local NvidiaAftermath_SDKPath = "../../../vendor/NVIDIA_Nsight_Aftermath_SDK"
local NvidiaAftermath_SDKEnabled = CommonConfig.PathExists(NvidiaAftermath_SDKPath)

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
        
        "%{IncludeDirs.InsightToolShaderCompiler}",

        "%{IncludeDirs.glfw}",
        "%{IncludeDirs.glm}",
        "%{IncludeDirs.imgui}",
        "%{IncludeDirs.reflect}",
        "%{IncludeDirs.zip}",
        "%{IncludeDirs.spdlog}",
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
        "%{IncludeDirs.vma_src}",
        "%{IncludeDirs.vma_inc}",
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
        "GLFW",
        "glm",
        "imgui",
        "zip",

        "D3d12",
        "DXGI",
        "dxguid",
        "dxcompiler",
        "meshoptimizer",
        "nvrhi",
        --"ShaderCompiler"
    }

    if VULKAN_SDK == nil then
    else
        links
        {
            "vulkan-1",
        }
    end

    if NvidiaAftermath_SDKEnabled then
        links
        {
            "GFSDK_Aftermath_Lib.x64",
        }
    end
end

function GraphicsConfig.FilterConfigurations()
    CommonConfig.FilterConfigurations()
    
    filter "configurations:Debug or configurations:Testing"
        links
        {
            "GenericCodeGen",
            "glslang",
            "glslang-default-resource-limits",
            "MachineIndependent",
            "OSDependent",
            "SPIRV",
            "SPIRV-Tools",
            "SPIRV-Tools-opt",
            "SPVRemapper",

            "ffx_fsr2_api_x64d",
            "ffx_fsr2_api_dx12_x64d",
        }
    if VULKAN_SDK == nil then
    else
        links
        {
            "ffx_fsr2_api_vk_x64d",
        }
    end

filter "configurations:Release"  
    links
    {
        "glslang",
        "MachineIndependent",
        "GenericCodeGen",
        "OSDependent",
        "TracyClient",

        "ffx_fsr2_api_x64",
        "ffx_fsr2_api_dx12_x64",
    }
    if VULKAN_SDK == nil then
    else
        links
        {
            "ffx_fsr2_api_vk_x64",
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
                "{COPYFILE} \"" .. renderDocPath .. "/renderdoc.dll\" \"%{wks.location}deps/".. outputdir..  "/dll/\"\n",
            }
        end
end

return GraphicsConfig