local CommonConfig = require "../lua/CommonConfig"

local CoreConfig = require "../Core/lua/CoreConfig"
local MathsConfig = require "../Maths/lua/MathsConfig"
local InputConfig = require "../Input/lua/InputConfig"
local GraphicsConfig = require "../Graphics/lua/GraphicsConfig"
local PhysicsConfig = require "../Physics/lua/PhysicsConfig"
local RuntimeConfig = require "../Runtime/lua/RuntimeConfig"

project "InsightStandalone"  
    configurations { "Debug", "Release" } 
    location "./"
    kind "ConsoleApp"

    targetname ("%{prj.name}" .. output_project_subfix)
    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
    debugdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")

    defines
    {
        "IS_MONOLITH",
        "IS_STANDALONE",
        "_CRT_SECURE_NO_WARNINGS",
        "GLM_FORCE_SWIZZLE",
        "GLM_FORCE_LEFT_HANDED",
        "GLM_FORCE_DEPTH_ZERO_TO_ONE",

        "IS_PLATFORM_X64",
        "IS_MEMORY_TRACKING",
        "RENDER_GRAPH_ENABLED",
        "TOBJECTPTR_REF_COUNTING",

        "COMMAND_LIST_RENDER_BATCH",
        "IS_NVIDIA_AFTERMATH_ENABLED",
    }

    files 
    { 
        "../../Engine/Core/inc/**.hpp", 
        "../../Engine/Core/inc/**.h",
        "../../Engine/Core/inc/**.inl",
        "../../Engine/Core/inc/**.cpp",
        "../../Engine/Core/src/**.cpp",
        "../../Engine/Core/src/**.inl",

        "../../Engine/Maths/inc/**.hpp", 
        "../../Engine/Maths/inc/**.h", 
        "../../Engine/Maths/inc/**.inl",
        "../../Engine/Maths/inc/**.cpp", 
        "../../Engine/Maths/src/**.cpp", 
        "../../Engine/Maths/src/**.inl",

        "../../Engine/Input/inc/**.hpp", 
        "../../Engine/Input/inc/**.h", 
        "../../Engine/Input/inc/**.inl",
        "../../Engine/Input/inc/**.cpp", 
        "../../Engine/Input/src/**.cpp", 
        "../../Engine/Input/src/**.inl",

        "../../Engine/Physics/inc/**.hpp", 
        "../../Engine/Physics/inc/**.h", 
        "../../Engine/Physics/inc/**.inl",
        "../../Engine/Physics/inc/**.cpp", 
        "../../Engine/Physics/src/**.cpp", 
        "../../Engine/Physics/src/**.inl",

        "../../Engine/Graphics//inc/**.hpp", 
        "../../Engine/Graphics/inc/**.h", 
        "../../Engine/Graphics/inc/**.inl",
        "../../Engine/Graphics/inc/**.cpp", 
        "../../Engine/Graphics/src/**.cpp", 
        "../../Engine/Graphics/src/**.inl",

        "../../Engine/Runtime/inc/**.hpp", 
        "../../Engine/Runtime/inc/**.h", 
        "../../Engine/Runtime/inc/**.inl",
        "../../Engine/Runtime/inc/**.cpp",
        "../../Engine/Runtime/src/**.cpp",
        "../../Engine/Runtime/src/**.inl",

        "inc/**.hpp", 
        "inc/**.h", 
        "inc/**.inl", 
        "inc/**.cpp",
        "src/**.cpp",
        "src/**.inl",

        --PROJECT_FILES

        "../../vendor/stb/stb_image.h",
        "../../vendor/stb/stb_image_write.h",
        "../../vendor/SPIRV-Reflect/spirv_reflect.h",
        "../../vendor/SPIRV-Reflect/spirv_reflect.cpp",
    }

    includedirs
    {
        "inc",
        "%{IncludeDirs.InsightCore}",
        "%{IncludeDirs.InsightMaths}",
        "%{IncludeDirs.InsightInput}",
        "%{IncludeDirs.InsightPhysics}",
        "%{IncludeDirs.InsightGraphics}",
        "%{IncludeDirs.InsightRuntime}",
    }

    CoreConfig.DefinesStaticLib()
    MathsConfig.DefinesStaticLib()
    InputConfig.DefinesStaticLib()
    GraphicsConfig.DefinesStaticLib()
    PhysicsConfig.DefinesStaticLib()
    RuntimeConfig.DefinesStaticLib()

    CoreConfig.IncludeDirs()
    MathsConfig.IncludeDirs()
    InputConfig.IncludeDirs()
    GraphicsConfig.IncludeDirs()
    PhysicsConfig.IncludeDirs()
    RuntimeConfig.IncludeDirs()

    CoreConfig.LibraryDirs()
    MathsConfig.LibraryDirs()
    InputConfig.LibraryDirs()
    GraphicsConfig.LibraryDirs()
    PhysicsConfig.LibraryDirs()
    RuntimeConfig.LibraryDirs()

    CoreConfig.LibraryLinks()
    MathsConfig.LibraryLinks()
    InputConfig.LibraryLinks()
    GraphicsConfig.LibraryLinks()
    PhysicsConfig.LibraryLinks()
    RuntimeConfig.LibraryLinks()

    CoreConfig.FilterConfigurations()
    MathsConfig.FilterConfigurations()
    InputConfig.FilterConfigurations()
    GraphicsConfig.FilterConfigurations()
    --PhysicsConfig.FilterConfigurations()
    RuntimeConfig.FilterConfigurations()

    CoreConfig.FilterPlatforms()
    MathsConfig.FilterPlatforms()
    InputConfig.FilterPlatforms()
    GraphicsConfig.FilterPlatforms()
    --PhysicsConfig.FilterPlatforms()
    RuntimeConfig.FilterPlatforms()

    removelinks
    {
        "Insight_Core" .. output_project_subfix .. ".lib",
        "Insight_Maths" .. output_project_subfix .. ".lib",
        "Insight_Input" .. output_project_subfix .. ".lib",
        "Insight_Graphics" .. output_project_subfix .. ".lib",
        "Insight_Physics" .. output_project_subfix .. ".lib",
        "Insight_Runtime" .. output_project_subfix .. ".lib",
    }
    CommonConfig.PostBuildCopyLibraryToOutput()

    filter { "system:Windows", "configurations:Debug" or "configurations:Testing" }
    ignoredefaultlibraries
    {
        "libcmt.lib",
        "msvcrt.lib",
        "libcmtd.lib",
    }
filter { "system:Windows", "configurations:Release" }
    ignoredefaultlibraries
    {
        "libcmt.lib",
        "libcmtd.lib",
        "msvcrtd.lib",
    }

filter "configurations:Testing"
    defines
    {
        --"TESTING",
        "TEST_ENABLED",
        --"DOCTEST_CONFIG_IMPLEMENTATION_IN_DLL",
    }
    files 
    { 
        "vendor/doctest/doctest/doctest.h",
    } 
    libdirs
    {
        "%{LibDirs.deps_testing_lib}",
    }