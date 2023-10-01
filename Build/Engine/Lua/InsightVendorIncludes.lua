-- simple modules

-- table for our functions
local InsightVendorIncludes = { }

local VendorIncludes = {}
VendorIncludes["doctest"] = "%{wks.location}vendor/doctest/doctest"
VendorIncludes["glfw"] = "%{wks.location}vendor/glfw/include"
VendorIncludes["glm"] = "%{wks.location}vendor/glm"
VendorIncludes["imgui"] = "%{wks.location}vendor/imgui"
VendorIncludes["implot"] = "%{wks.location}vendor/implot"
VendorIncludes["spdlog"] = "%{wks.location}vendor/spdlog/include"
VendorIncludes["vma"] = "%{wks.location}vendor/VulkanMemoryAllocator/src"
VendorIncludes["vulkan"] = os.getenv("VULKAN_SDK") .. "/include/"
VendorIncludes["spirv_reflect"] = "%{wks.location}vendor/SPIRV-Reflect"
VendorIncludes["assimp"] = "%{wks.location}vendor/assimp/include"
VendorIncludes["tracy"] = "%{wks.location}vendor/tracy"
VendorIncludes["pix"] = "%{wks.location}vendor/winpixeventruntime/Include"
VendorIncludes["stb_image"] = "%{wks.location}vendor/stb"
VendorIncludes["meshoptimizer"] = "%{wks.location}vendor/meshoptimizer/src"
VendorIncludes["FFR2"] = "%{wks.location}vendor/FidelityFX-FSR2/src"
VendorIncludes["Aftermath"] = "%{wks.location}vendor/NVIDIA_Nsight_Aftermath_SDK_2022.1.0.22080/include"
VendorIncludes["nvtx"] = "%{wks.location}vendor/NVTX/c/include"
VendorIncludes["xxHash"] = "%{wks.location}vendor/xxHash"
VendorIncludes["splash"] = "%{wks.location}vendor/Splash/Splash/inc"
VendorIncludes["DirectXHeaders"] = "%{wks.location}vendor/DirectX-Headers/include/directx"
VendorIncludes["DirectXAgilityHeaders"] = "%{wks.location}vendor/Microsoft.Direct3D.D3D12/build/native/include"
VendorIncludes["DirectXShaderCompiler"] = "%{wks.location}vendor/DirectXShaderCompiler/inc"
VendorIncludes["D3D12MemoryAllocator"] = "%{wks.location}vendor/D3D12MemoryAllocator/include"
VendorIncludes["IconFontCppHeaders"] = "%{wks.location}vendor/IconFontCppHeaders"
VendorIncludes["nlohmann_json"] = "%{wks.location}vendor/nlohmann_json/single_include"
VendorIncludes["lz4"] = "%{wks.location}vendor/lz4/lib"
VendorIncludes["qoi"] = "%{wks.location}vendor/qoi"
VendorIncludes["reflect"] = "%{wks.location}vendor/reflect/reflect/inc"
VendorIncludes["simplygon"] = "%{wks.location}vendor/SimplygonSDK_10.1.11000.0"
VendorIncludes["zip"] = "%{wks.location}vendor/zip/src"
VendorIncludes["efsw"] = "%{wks.location}vendor/efsw/include"

-- Shorthand form is less typing and doesn't use a local variable
function InsightVendorIncludes.AddIncludes(includeTable)
    for k,v in pairs(VendorIncludes) 
        do includeTable[k] = v 
    end
end

function InsightVendorIncludes.AddIncludesToIncludeDirs(includeDirs)
    for k,v in pairs(VendorIncludes) 
        do includeDirs { v }
    end
end

return InsightVendorIncludes