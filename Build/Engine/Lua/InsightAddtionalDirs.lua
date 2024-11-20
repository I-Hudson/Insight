-- table for our functions
local InsightAdditionalDirs = { }

function InsightAdditionalDirs.All()
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
end

return InsightAdditionalDirs