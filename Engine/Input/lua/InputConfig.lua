local CommonConfig = require "../../lua/CommonConfig"

local InputConfig = { }

function InputConfig.DefinesSharedLib()
    defines
    {
        "IS_EXPORT_INPUT_DLL",
    }
end

function InputConfig.DefinesStaticLib()
    defines
    {
    }
end

function InputConfig.IncludeDirs()
    includedirs
    {
        "inc",
        "%{IncludeDirs.InsightCore}",
        "%{IncludeDirs.InsightMaths}",

        "%{IncludeDirs.imgui}",
        "%{IncludeDirs.spdlog}",
    }
end

function InputConfig.LibraryDirs()
    libdirs
    {
        --"%{wks.location}/deps/lib",
    }
end

function InputConfig.LibraryLinks()
    links
    {
        "Insight_Core" .. output_project_subfix .. ".lib",
        "Insight_Maths" .. output_project_subfix .. ".lib",

        "imgui.lib",
    }
    if (profileTool == "pix") then
        links
        {
            "WinPixEventRuntime.lib"
        }
    end
end

function InputConfig.FilterConfigurations()
    CommonConfig.FilterConfigurations()
end

function InputConfig.FilterPlatforms(AMD_Ryzen_Master_SDK, OutputDir)
    filter "platforms:Win64"
        links
        {
            "Xinput.lib",
            "WindowsApp.lib",
        }
end

return InputConfig