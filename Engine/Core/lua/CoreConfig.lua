local CommonConfig = require "../../lua/CommonConfig"

local CoreConfig = { }

function CoreConfig.DefinesSharedLib()
    defines
    {
        "IS_EXPORT_CORE_DLL",
    }
end

function CoreConfig.DefinesStaticLib()
    defines { }
end

function CoreConfig.IncludeDirs()
    includedirs
    {
        "inc",
        "%{IncludeDirs.spdlog}",
        "%{IncludeDirs.imgui}",
        "%{IncludeDirs.xxHash}",
        "%{IncludeDirs.glm}",
        "%{IncludeDirs.lz4}",
        "%{IncludeDirs.zip}",
        "%{IncludeDirs.nlohmann_json}",
        "%{IncludeDirs.implot}",
    }
end

function CoreConfig.LibraryDirs()
    libdirs
    {
        "%{wks.location}/deps/lib",
    }
end

function CoreConfig.LibraryLinks()
    links
    {
        "imgui.lib",
        "glm.lib",
        "zip.lib",
    }

    if (profileTool == "pix") then
        links
        {
            "WinPixEventRuntime.lib"
        }
    end
end

function CoreConfig.FilterConfigurations()

    CommonConfig.FilterConfigurations()
    filter "configurations:Debug or configurations:Testing"
        links
        {
            "xxHashd.lib",
            "lz4d.lib",
        }

    filter "configurations:Release"  
        links
        {
            "xxHash.lib",
            "lz4.lib",
        }
end

function CoreConfig.FilterPlatforms(AMD_Ryzen_Master_SDK, OutputDir)
    filter "platforms:Win64"
        if AMD_Ryzen_Master_SDK == nil then
        else
            defines
            {
                "AMD_Ryzen_Master_SDK",
            }
            includedirs
            {
                AMD_Ryzen_Master_SDK .. "/include",
            }
            links
            {
                "Platform.lib",
                "Device.lib",
            }
            prebuildcommands
            {
                "{COPY} \"" .. AMD_Ryzen_Master_SDK .. "/bin/Platform.dll\" \"%{wks.location}deps/".. OutputDir..  "/dll/\"\n",
                "{COPY} \"" .. AMD_Ryzen_Master_SDK .. "/lib/Platform.lib\" \"%{wks.location}deps/".. OutputDir..  "/lib/\"\n",
                "{COPY} \"" .. AMD_Ryzen_Master_SDK .. "/bin/Device.dll\" \"%{wks.location}deps/".. OutputDir..  "/dll/\"\n",
                "{COPY} \"" .. AMD_Ryzen_Master_SDK .. "/lib/Device.lib\" \"%{wks.location}deps/".. OutputDir..  "/lib/\"\n",
            }
        end
        links
        {
            "Ole32.lib",
            "dbghelp.lib",
            "Rpcrt4.lib",
        }
end

return CoreConfig