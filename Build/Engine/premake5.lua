local InsightEngineIncludes = require "lua/InsightEngineIncludes"
local InsightVendorIncludes = require "lua/InsightVendorIncludes"

local profileTool="tracy"
local monolith_build="false"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
outputdir_target = "%{wks.location}bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
outputdir_obj = "%{wks.location}bin-int/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
outputdir_debug = "%{wks.location}bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
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

IncludeDirs = {}
InsightEngineIncludes.AddIncludes(IncludeDirs, "%{wks.location}")
InsightVendorIncludes.AddIncludes(IncludeDirs, "%{wks.location}")

LibDirs = {}
LibDirs["deps_lib"] = "%{wks.location}deps/" .. outputdir .. "/lib/"
LibDirs["deps_testing_lib"] = "%{wks.location}deps/Debug-windows-x86_64/lib/"

LibDirs["imgui"] = "%{wks.location}vendor/imgui/" .. outputdir .. "ImGui/"
LibDirs["vulkan"] = VULKAN_SDK .. "/lib/"

workspace "Insight"
    architecture "x64"
    startproject "InsightEditor"
    staticruntime "off"
    location "../../"

    language ("C++")
    cppdialect ("C++17")

    configurations
    {
        "Debug",
        "Release",

        "Testing"
    }
    platforms 
    { 
        "Win64",
        "UWP",
    }

    flags
    {
    	"MultiProcessorCompile"
    }

    defines
    {
        "_CRT_SECURE_NO_WARNINGS",
        "_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS",
        "_SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING",
        "_SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS",

        "GLM_FORCE_SWIZZLE",
        "GLM_FORCE_LEFT_HANDED",
        "GLM_FORCE_DEPTH_ZERO_TO_ONE",

        "REFLECT_TYPE_INFO_ENABLED",
        "REFLECT_DLL_IMPORT",

        "IS_PLATFORM_X64",
        "IS_MEMORY_TRACKING",
        "IS_ENGINE",
        "RENDER_GRAPH_ENABLED",
        "TOBJECTPTR_REF_COUNTING",

        --"RENDERGRAPH_V2_ENABLED",
        "IS_RESOURCE_HANDLES_ENABLED",
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

    if (monolith_build == "false") then
        defines{ "IS_EXPORT_DLL", }
        kind "SharedLib"
        table.insert(post_build_commands, "{COPY} \"%{cfg.targetdir}/%{prj.name}" .. output_project_subfix .. ".dll\" \"%{wks.location}deps/".. outputdir..  "/dll/\"\n")
        table.insert(post_build_commands, "{COPY} \"%{cfg.targetdir}/%{prj.name}" .. output_project_subfix .. ".lib\" \"%{wks.location}deps/".. outputdir..  "/lib/\"\n")
        table.insert(post_build_commands, "{COPY} \"%{cfg.targetdir}/%{prj.name}" .. output_project_subfix .. ".pdb\" \"%{wks.location}deps/".. outputdir..  "/pdb/\"\n")
        table.insert(post_build_commands, "{COPY} \"%{cfg.targetdir}/%{prj.name}" .. output_project_subfix .. ".dll\" \"%{wks.location}bin/".. outputdir..  "/" .. output_executable .. "/\"\n")
    end
    if (monolith_build == "true") then
        defines { "IS_MONOLITH" }
        kind "StaticLib"
        output_project_subfix = "_monolith"
        table.insert(post_build_commands, "{COPYFILE} \"%{cfg.targetdir}/%{prj.name}" .. output_project_subfix .. ".lib\" \"%{wks.location}deps/".. outputdir..  "/lib/\"")
    end

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
            "_DEBUG",
            "IS_TEST"
        }

    filter "configurations:Release"
        buildoptions "/MD"
        defines
        {
            "NDEBUG",
            "IS_RELEASE",
            "DOCTEST_CONFIG_DISABLE",
        }

    filter "system:windows"


    filter { "platforms:Win64" or "platforms:UWP" }
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
            "Comctl32.lib",
        }

    filter { "platforms:Win64", "configurations:Debug" or "configurations:Testing" }
        ignoredefaultlibraries
        {
            "libcmt.lib",
            "msvcrt.lib",
            "libcmtd.lib",
        }
    filter { "platforms:Win64", "configurations:Release" }
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
    	
    
    filter "system:Unix"
    	system "linux"
    	toolset("clang")
        defines
        {
            "IS_PLATFORM_LINUX",
            "IS_VULKAN_ENABLED",
        }

group "Editor"
    include "../../Engine/Editor/Editor.lua"

group "Runtime"
    include "../../Engine/Core/Core.lua"
    include "../../Engine/Maths/Maths.lua"
    include "../../Engine/Graphics/Graphics.lua"
    include "../../Engine/Input/Input.lua"
    include "../../Engine/Runtime/Runtime.lua"

newaction{
    trigger = "clean",
    description = "Remove all binaries and intermediate binaries, and vs files.",
    execute = function()
        print("Removing symlinks")
        os.execute("Remove_Symlinks.bat")
        print("Removeing binaries")
        os.rmdir("../../bin")
        print("Removeing internediate binaries")
        os.rmdir("../../bin-int")
        print("Removing dependencies")
        os.rmdir("../../deps")
        print("Removeing project files")
        os.rmdir("../../.vs")
        os.remove("../../Engine/**.sln")
        os.remove("../../Engine/**.vcxproj")
        os.remove("../../Engine/**.vcxproj.filters")
        os.remove("../../Engine/**.vcxproj.user")
        print("Done")
    end
} 