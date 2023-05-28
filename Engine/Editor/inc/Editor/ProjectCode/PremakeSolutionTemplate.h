#include <string>

namespace Insight::Editor
{
    constexpr static const char* c_PremakeSolutionTag_SolutionName = "--SOLUTION_NAME";
    constexpr static const char* c_PremakeSolutionTag_ProjectName = "--PROJECT_NAME";
    constexpr static const char* c_PremakeSolutionTag_InsightPath = "--INSIGHT_PATH";
    constexpr static const char* c_PremakeSolutionTag_SolutionLocation = "--SOLUTION_LOCATION";
    constexpr static const char* c_PremakeSolutionTag_PremakeProjectFileName = "--PREMAKE_PROJECT_FILE_NAME";
    
    constexpr static const char* c_PremakeSolutionFileName = "premake5_solution.lua";
    
    constexpr static const char* c_PremakeSolutionTemplate = R"(

local profileTool="tracy"
local monolith_build="false"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
outputdir_target = "%{insightPath}bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
outputdir_obj = "%{insightPath}bin-int/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
outputdir_debug = "%{insightPath}bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
output_executable = "Insight_Editor"
output_project_subfix = ""

post_build_commands = {}
function concat_table(table_to_concat)
    return table.concat(table_to_concat, " ")
end

function tprint (tbl, indent)
    if not indent then indent = 0 end
    local toprint = string.rep(" ", indent) .. "{\r\n"
    indent = indent + 2 
    for k, v in pairs(tbl) do
      toprint = toprint .. string.rep(" ", indent)
      if (type(k) == "number") then
        toprint = toprint .. "[" .. k .. "] = "
      elseif (type(k) == "string") then
        toprint = toprint  .. k ..  "= "   
      end
      if (type(v) == "number") then
        toprint = toprint .. v .. ",\r\n"
      elseif (type(v) == "string") then
        toprint = toprint .. "\"" .. v .. "\",\r\n"
      elseif (type(v) == "table") then
        toprint = toprint .. tprint(v, indent + 2) .. ",\r\n"
      else
        toprint = toprint .. "\"" .. tostring(v) .. "\",\r\n"
      end
    end
    toprint = toprint .. string.rep(" ", indent-2) .. "}"
    return toprint
  end

VULKAN_SDK = os.getenv("VULKAN_SDK")
insightPath = "--INSIGHT_PATH"

IncludeDirs = {}
IncludeDirs["InsightCore"] = "%{insightPath}Engine/Core/inc"
IncludeDirs["InsightMaths"] = "%{insightPath}Engine/Maths/inc"
IncludeDirs["InsightGraphics"] = "%{insightPath}Engine/Graphics/inc"
IncludeDirs["InsightInput"] = "%{insightPath}Engine/Input/inc"
IncludeDirs["InsightRuntime"] = "%{insightPath}Engine/Runtime/inc"
IncludeDirs["InsightEditor"] = "%{insightPath}Engine/Editor/inc"

IncludeDirs["doctest"] = "%{insightPath}vendor/doctest/doctest"
IncludeDirs["glfw"] = "%{insightPath}vendor/glfw/include"
IncludeDirs["glm"] = "%{insightPath}vendor/glm"
IncludeDirs["imgui"] = "%{insightPath}vendor/imgui"
IncludeDirs["implot"] = "%{insightPath}vendor/implot"
IncludeDirs["spdlog"] = "%{insightPath}vendor/spdlog/include"
IncludeDirs["vma"] = "%{insightPath}vendor/VulkanMemoryAllocator/src"
IncludeDirs["vulkan"] = VULKAN_SDK .. "/include/"
IncludeDirs["spirv_reflect"] = "%{insightPath}vendor/SPIRV-Reflect"
IncludeDirs["assimp"] = "%{insightPath}vendor/assimp/include"
IncludeDirs["tracy"] = "%{insightPath}vendor/tracy"
IncludeDirs["pix"] = "%{insightPath}vendor/winpixeventruntime/Include"
IncludeDirs["stb_image"] = "%{insightPath}vendor/stb"
IncludeDirs["meshoptimizer"] = "%{insightPath}vendor/meshoptimizer/src"
IncludeDirs["FFR2"] = "%{insightPath}vendor/FidelityFX-FSR2/src"
IncludeDirs["Aftermath"] = "%{insightPath}vendor/NVIDIA_Nsight_Aftermath_SDK_2022.1.0.22080/include"
IncludeDirs["nvtx"] = "%{insightPath}vendor/NVTX/c/include"
IncludeDirs["xxHash"] = "%{insightPath}vendor/xxHash"
IncludeDirs["splash"] = "%{insightPath}vendor/Splash/Splash/inc"
IncludeDirs["DirectXHeaders"] = "%{insightPath}vendor/DirectX-Headers/include/directx"
IncludeDirs["DirectXAgilityHeaders"] = "%{insightPath}vendor/Microsoft.Direct3D.D3D12/build/native/include"
IncludeDirs["DirectXShaderCompiler"] = "%{insightPath}vendor/DirectXShaderCompiler/inc"
IncludeDirs["D3D12MemoryAllocator"] = "%{insightPath}vendor/D3D12MemoryAllocator/include"
IncludeDirs["IconFontCppHeaders"] = "%{insightPath}vendor/IconFontCppHeaders"
IncludeDirs["nlohmann_json"] = "%{insightPath}vendor/nlohmann_json/single_include"
IncludeDirs["lz4"] = "%{insightPath}vendor/lz4/lib"
IncludeDirs["qoi"] = "%{insightPath}vendor/qoi"
IncludeDirs["reflect"] = "%{insightPath}vendor/reflect/reflect/inc"
IncludeDirs["simplygon"] = "%{insightPath}vendor/SimplygonSDK_10.1.11000.0"

LibDirs = {}
LibDirs["deps_lib"] = "%{insightPath}deps/" .. outputdir .. "/lib/"
LibDirs["deps_testing_lib"] = "%{insightPath}deps/Debug-windows-x86_64/lib/"

LibDirs["imgui"] = "%{insightPath}vendor/imgui/" .. outputdir .. "ImGui/"
LibDirs["vulkan"] = VULKAN_SDK .. "/lib/"

workspace "--SOLUTION_NAME"
    architecture "x64"
    startproject "--PROJECT_NAME"
    staticruntime "off"
    location "--SOLUTION_LOCATION"

    language ("C++")
    cppdialect ("C++17")


    configurations
    {
        "Debug",
        "Release",
    }
    platforms 
    { 
        "Win64",
    }

    flags
    {
    	"MultiProcessorCompile"
    }

    defines
    {
        "_CRT_SECURE_NO_WARNINGS",
        "GLM_FORCE_SWIZZLE",
        "GLM_FORCE_LEFT_HANDED",
        "GLM_FORCE_DEPTH_ZERO_TO_ONE",

        "IS_PLATFORM_X64",
        "IS_MEMORY_TRACKING",
        "RENDER_GRAPH_ENABLED",
        "TOBJECTPTR_REF_COUNTING",
    }

    includedirs
    {
        "%{IncludeDirs.tracy}",
        "%{IncludeDirs.doctest}",
        "%{IncludeDirs.pix}",
        "%{IncludeDirs.IconFontCppHeaders}",
        "%{IncludeDirs.nlohmann_json}",
        "%{IncludeDirs.implot}",
    }

    libdirs
    {
        "%{LibDirs.deps_lib}",
    }

    if (profileTool == "tracy") then
        editandcontinue "off"
        defines { "IS_PROFILE_ENABLED", "IS_PROFILE_TRACY", "TRACY_IMPORTS", "TRACY_ON_DEMAND", }
        filter "configurations:Debug or configurations:Testing"
            links { "tracy.lib", }
        filter { "configurations:Release" }
            links { "tracy.lib", }
    end
    if (profileTool == "pix") then
        defines { "IS_PROFILE_ENABLED", "IS_PROFILE_PIX", "USE_PIX" }
    end

    filter "configurations:Debug"
        defines
        {
            "DOCTEST_CONFIG_DISABLE",
            "IS_DEBUG",
        }

    filter { "configurations:Debug", "configurations:Testing" }
        buildoptions "/MDd"
        defines
        {
            "_DEBUG"
        }

    filter "configurations:Release"
        buildoptions "/MD"
        defines
        {
            "NDEBUG",
            "IS_RELEASE",
            "DOCTEST_CONFIG_DISABLE",
        }

    filter "system:Windows"
    	system "windows"
    	toolset("msc-v143")
        defines
        {
            "IS_PLATFORM_WINDOWS",
            "IS_PLATFORM_WIN32",
            --"IS_MATHS_DIRECTX_MATHS",
            "IS_DX12_ENABLED",
            "IS_VULKAN_ENABLED",
            "IS_CPP_WINRT",
            
            "USE_PIX",
            "NOMINMAX",

            "SPLASH_PLATFORM_WINDOWS",
            
            "VK_USE_PLATFORM_WIN32_KHR",
        }
        links
        {
            "WinPixEventRuntime.lib",
            "cppwinrt_fast_forwarder.lib",
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
            "IS_TESTING",
            "TEST_ENABLED",
        }
        files 
        { 
            "vendor/doctest/doctest/doctest.h",
        } 
        libdirs
        {
            "%{LibDirs.deps_testing_lib}",
        }

    include "--PREMAKE_PROJECT_FILE_NAME"
    )";

    struct PremakeSolutionTemplateData
    {
        const char* SolutionName;
        const char* ProjectName;
        const char* InsightRootPath;
    };
    std::string CreatePremakeSolutionTemplateFile(const char* outFolder, const PremakeSolutionTemplateData& templateData);
}