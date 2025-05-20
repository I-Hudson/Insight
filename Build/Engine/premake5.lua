local InsightEngineIncludes = require "lua/InsightEngineIncludes"
local InsightVendorIncludes = require "lua/InsightVendorIncludes"

local InsightDefines = require "lua/InsightDefines"
local InsightAddtionalDirs = require "lua/InsightAddtionalDirs"
local InsightConfigurations = require "lua/InsightConigurations"
local InsightPlatforms = require "lua/InsightPlatforms"

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

  IncludeDirs = {}
  InsightEngineIncludes.AddIncludes(IncludeDirs, "%{wks.location}")
  InsightEngineIncludes.AddToolsIncludes(IncludeDirs, "%{wks.location}")
  InsightVendorIncludes.AddIncludes(IncludeDirs, "%{wks.location}")
  
  LibDirs = {}
  LibDirs["deps_lib"] = "%{wks.location}deps/" .. outputdir .. "/lib/"
  LibDirs["deps_testing_lib"] = "%{wks.location}deps/Debug-windows-x86_64/lib/"
  
  LibDirs["imgui"] = "%{wks.location}vendor/imgui/" .. outputdir .. "ImGui/"
  
  VULKAN_SDK = os.getenv("VULKAN_SDK")
  if VULKAN_SDK == nil then
  else
    LibDirs["vulkan"] = VULKAN_SDK .. "/lib/"
    end

workspace "Insight"
    architecture "x64"
    startproject "InsightEditor"
    staticruntime "off"
    location "../../"

    language ("C++")
    cppdialect ("C++17")
    enableunitybuild "off"

    configurations
    {
        "Debug",
        "Release",

        "Testing"
    }
    platforms 
    { 
        "Win64",
        --"UWP",
    }

    flags
    {
    	"MultiProcessorCompile"
    }

    InsightDefines.All()
    InsightAddtionalDirs.All()

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

    InsightConfigurations.All()
    InsightPlatforms.All()

group "Editor"
    include "../../Engine/Editor/Editor.lua"

group "Runtime"
    include "../../Engine/Core/Core.lua"
    include "../../Engine/Maths/Maths.lua"
    include "../../Engine/Physics/Physics.lua"
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