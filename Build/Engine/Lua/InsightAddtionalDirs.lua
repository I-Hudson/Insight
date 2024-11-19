-- table for our functions
local InsightAddtionalDirs = { }

function InsightAddtionalDirs.All()
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

return InsightAddtionalDirs