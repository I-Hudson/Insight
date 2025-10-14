local CommonConfig = require "../../lua/CommonConfig"

local FileSystemConfig = { }

function FileSystemConfig.DefinesSharedLib()
    defines
    {
        "IS_EXPORT_FILESYSTEM_DLL",
    }
end

function FileSystemConfig.DefinesStaticLib()
    defines
    {
    }
end

function FileSystemConfig.IncludeDirs()
    includedirs
    {
        "inc",
        "%{IncludeDirs.InsightCore}",
        
        "%{IncludeDirs.spdlog}",
    }
end

function FileSystemConfig.LibraryDirs()
    libdirs
    {
        --"%{wks.location}/deps/lib",
    }
end

function FileSystemConfig.LibraryLinks()
    links
    {
        "Insight_Core" .. output_project_subfix .. ".lib",
    }
    if (profileTool == "pix") then
        links
        {
            "WinPixEventRuntime.lib"
        }
    end
end

function FileSystemConfig.FilterConfigurations()
    CommonConfig.FilterConfigurations()
end

function FileSystemConfig.FilterPlatforms()
end

return FileSystemConfig