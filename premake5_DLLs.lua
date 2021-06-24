workspace "Insight_DLLs"
    architecture "x64"

    configurations
    {
        "Debug",
        "Release"
    }
    
    flags
	{
		"MultiProcessorCompile"
    }
    
    defines
    {
        "IS_PROFILE",
        --"IS_PROFILE_OPTICK",
        "_CRT_SECURE_NO_WARNINGS",
    }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

group "Insight_DLLs_Deps"
    include "Insight/InsightGraphics/vendor/glad/premake5.lua"
    include "Insight/InsightGraphics/vendor/GLFW/premake5.lua"
    include "Insight/InsightGraphics/vendor/glslang/glslang/OSDependent/Windows/premake5.lua"
    include "Insight/InsightGraphics/vendor/glslang/StandAlone/premake5.lua"
    include "Insight/InsightGraphics/vendor/glslang/glslang/premake5.lua"
    include "Insight/InsightGraphics/vendor/glslang/OGLCompilersDLL/premake5.lua"
    include "Insight/InsightGraphics/vendor/glslang/SPIRV/premake5.lua"
group "Insight_DLLs"
    include "Insight/InsightCore/premake5.lua"
    include "Insight/InsightGraphics/premake5.lua"