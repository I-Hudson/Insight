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
        "Insight_Core" .. output_project_subfix .. "",
        "Insight_Maths" .. output_project_subfix .. "",

        "imgui",
    }
    if (profileTool == "pix") then
        links
        {
            "WinPixEventRuntime"
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
            "Xinput",
            "WindowsApp",
        }
end

return InputConfig