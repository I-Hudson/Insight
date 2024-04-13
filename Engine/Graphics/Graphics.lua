-- Check if a file or directory exists in this path
function fileExists(file)
    local ok, err, code = os.rename(file, file)
    if not ok then
       if code == 13 then
          --print "Permission denied, but it exists"
          return true
       elseif code == 5 then
        --print "Permission denied, but it exists"
        return true
       end
    end
    return false
 end
 
 --- Check if a directory exists in this path
 function isdir(path)
    -- "/" works on both Unix and Windows
    local result = fileExists(path)
    return result
 end

local local_post_build_commands = post_build_commands

project "Insight_Graphics"  
    configurations { "Debug", "Release" } 
    location "./"

    targetname ("%{prj.name}" .. output_project_subfix)
    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
    debugdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")

    dependson 
    {
        "Insight_Core",
        "Insight_Maths",
        "Insight_Input",
    }

    defines
    {
        "IS_EXPORT_GRAPHICS_DLL",
        "COMMAND_LIST_RENDER_BATCH",
    }

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
    }

    files 
    { 
        "inc/**.hpp", 
        "inc/**.h", 
        "inc/**.inl", 
        "src/**.cpp",
        "src/**.inl",

        "../Core/inc/Memory/NewDeleteOverload.h", 
        "../Core/src/Memory/NewDeleteOverload.cpp",

        "../../vendor/SPIRV-Reflect/spirv_reflect.h",
        "../../vendor/SPIRV-Reflect/spirv_reflect.cpp",
        "../../vendor/stb/stb_image.h",
        "../../vendor/stb/stb_image_write.h",
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
        "zip.lib",

        "D3d12.lib",
        "DXGI.lib",
        "dxguid.lib",
        "dxcompiler.lib",
        "meshoptimizer.lib",
    }
    
    libdirs
    {
        "%{LibDirs.vulkan}",
    }

    if VULKAN_SDK == nil then
    else
        links
        {
            "vulkan-1.lib",
        }
    end
    
    if os.rename("../../vendor/NVIDIA_Nsight_Aftermath_SDK", "../../vendor/NVIDIA_Nsight_Aftermath_SDK") then
        defines
        {
            "IS_NVIDIA_AFTERMATH_ENABLED",
        }
        links
        {
            "GFSDK_Aftermath_Lib.x64.lib",
        }
    end

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

    filter "platforms:Win64"
        local renderDocPath = "C:/Program Files/RenderDoc"
        if isdir(renderDocPath) then
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

    filter "configurations:Debug or configurations:Testing"
        defines { "DEBUG" }
        symbols "On" 
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
            "assimpd.lib",

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
        defines { "NDEBUG" }    
        optimize "On"   
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