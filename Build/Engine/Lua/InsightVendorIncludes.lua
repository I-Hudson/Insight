-- simple modules

-- table for our functions
local InsightVendorIncludes = { }

local VendorIncludes = {}
VendorIncludes["doctest"] = "vendor/doctest/doctest"
VendorIncludes["glfw"] = "vendor/glfw/include"
VendorIncludes["glm"] = "vendor/glm"
VendorIncludes["imgui"] = "vendor/imgui"
VendorIncludes["ImGuizmo"] = "vendor/ImGuizmo"
VendorIncludes["implot"] = "vendor/implot"
VendorIncludes["spdlog"] = "vendor/spdlog/include"
VendorIncludes["vma"] = "vendor/VulkanMemoryAllocator/src"
VendorIncludes["spirv_reflect"] = "vendor/SPIRV-Reflect"
VendorIncludes["assimp"] = "vendor/assimp/include"
VendorIncludes["tracy"] = "vendor/tracy"
VendorIncludes["pix"] = "vendor/winpixeventruntime/Include"
VendorIncludes["stb_image"] = "vendor/stb"
VendorIncludes["meshoptimizer"] = "vendor/meshoptimizer/src"
VendorIncludes["FFR2"] = "vendor/FidelityFX-FSR2/src"
VendorIncludes["Aftermath"] = "vendor/NVIDIA_Nsight_Aftermath_SDK/include"
VendorIncludes["nvtx"] = "vendor/NVTX/c/include"
VendorIncludes["xxHash"] = "vendor/xxHash"
VendorIncludes["splash"] = "vendor/Splash/Splash/inc"
VendorIncludes["DirectXHeaders"] = "vendor/DirectX-Headers/include/directx"
VendorIncludes["DirectXAgilityHeaders"] = "vendor/Microsoft.Direct3D.D3D12/build/native/include"
VendorIncludes["DirectXShaderCompiler"] = "vendor/DirectXShaderCompiler/inc"
VendorIncludes["D3D12MemoryAllocator"] = "vendor/D3D12MemoryAllocator/include"
VendorIncludes["IconFontCppHeaders"] = "vendor/IconFontCppHeaders"
VendorIncludes["nlohmann_json"] = "vendor/nlohmann_json/single_include"
VendorIncludes["lz4"] = "vendor/lz4/lib"
VendorIncludes["qoi"] = "vendor/qoi"
VendorIncludes["reflect"] = "vendor/reflect/reflect/inc"
VendorIncludes["simplygon"] = "vendor/SimplygonSDK_10.1.11000.0"
VendorIncludes["zip"] = "vendor/zip/src"
VendorIncludes["efsw"] = "vendor/efsw/include"

-- Shorthand form is less typing and doesn't use a local variable
function InsightVendorIncludes.AddIncludes(includeTable, prefix)
    for k,v in pairs(VendorIncludes) 
        do 
            includeTable[k] = prefix .. v 
    end
    local VULKAN_SDK = os.getenv("VULKAN_SDK")
    if VULKAN_SDK == nil then
    else
        includeTable["vulkan"] = VULKAN_SDK .. "/include/"
    end
end

function InsightVendorIncludes.AddIncludesToIncludeDirs(includeDirs)
    for k,v in pairs(VendorIncludes) 
        do includeDirs { v }
    end
    local VULKAN_SDK = os.getenv("VULKAN_SDK")
    if VULKAN_SDK == nil then
    else
        includeDirs { VULKAN_SDK .. "/include/" }
    end
   
end

return InsightVendorIncludes