-- table for our functions
local InsightConfigurations = { }

function InsightConfigurations.All()
    filter "configurations:Debug"
    buildoptions "/MDd"
    defines
    {
        "DOCTEST_CONFIG_DISABLE",
        "IS_DEBUG",
    }

    filter "configurations:Release"
    buildoptions "/MD"
    defines
    {
        "NDEBUG",
        "IS_RELEASE",
        "DOCTEST_CONFIG_DISABLE",
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
    includedirs
    {
        "../../vendor/glm",
    }
    libdirs
    {
        "%{LibDirs.deps_testing_lib}",
    }
end

return InsightConfigurations