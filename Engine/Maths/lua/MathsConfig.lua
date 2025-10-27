local CommonConfig = require "../../lua/CommonConfig"

local MathsConfig = { }

function MathsConfig.DefinesSharedLib()
    defines
    {
        "IS_EXPORT_MATHS_DLL",
        --"IS_MATHS_DIRECTX_MATHS",
        --"IS_MATHS_GLM",
    }
end

function MathsConfig.DefinesStaticLib()
    defines
    {
        --"IS_MATHS_DIRECTX_MATHS",
        --"IS_MATHS_GLM",
    }
end

function MathsConfig.IncludeDirs()
    includedirs
    {
        "inc",
        --"../../vendor/glm",
        "%{IncludeDirs.spdlog}",
        "%{IncludeDirs.imgui}",
        "%{IncludeDirs.glm}",
        "%{IncludeDirs.reflect}",
    }
end

function MathsConfig.LibraryDirs()
    libdirs
    {
        "%{wks.location}/deps/lib",
    }
end

function MathsConfig.LibraryLinks()
    links
    {
        "imgui",
        "glm"
    }
    if (profileTool == "pix") then
        links
        {
            "WinPixEventRuntime"
        }
    end
end

function MathsConfig.FilterConfigurations()
    CommonConfig.FilterConfigurations()

    filter "configurations:Testing"
    defines
    {
        "GLM_ENABLE_EXPERIMENTAL",
    }
    includedirs
    {
        "../../vendor/glm",
    }
end

function MathsConfig.FilterPlatforms(AMD_Ryzen_Master_SDK, OutputDir)
    filter "platforms:Win64"
        links
        {
            "Ole32",
            "dbghelp",
            "Rpcrt4",
        }
end

return MathsConfig