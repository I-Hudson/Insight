project "InsightGraphics"  
    kind "SharedLib"   
    language "C++"
    cppdialect "C++17"
    configurations { "Debug", "Release" } 

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
    debugdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")

    defines
    {
        "IS_EXPORT_GRAPHICS_DLL"
    }
    
    includedirs
    {
        "inc",
        "%{IncludeDirs.InsightCore}",

        "%{IncludeDirs.glfw}",
        "%{IncludeDirs.glm}",
        "%{IncludeDirs.stb}",
        "%{IncludeDirs.spdlog}",
        "%{IncludeDirs.vma}",
        "%{IncludeDirs.glslang}win_debug/inc",
        "%{IncludeDirs.vulkan}",
    }

    files 
    { 
        "inc/**.hpp", 
        "inc/**.h", 
        "src/**.cpp" 
    }

    links
    {
        "InsightCore",
        "glfw3.lib",
        "glm",
        "vulkan-1.lib",
    }

    libdirs
    {
        "%{wks.location}/vendor/glfw/lib",
        "%{LibDirs.vulkan}",
    }

    postbuildcommands
    {
       "{COPY} \"%{cfg.targetdir}/InsightGraphics.dll\" \"%{wks.location}/bin/".. outputdir..  "/InsightEditor/\"",
    }

    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On" 

        links
        {
            "GenericCodeGend.lib",
            "glslangd.lib",
            "glslang-default-resource-limitsd.lib",
            "HLSLd.lib",
            "MachineIndependentd.lib",
            "OGLCompilerd.lib",
            "OSDependentd.lib",
            "SPIRVd.lib",
            "SPIRV-Toolsd.lib",
            "SPIRV-Tools-optd.lib",
            "SPVRemapperd.lib",
        }

        libdirs
        {
            "%{LibDirs.glslang_win_d}",
        }

    filter "configurations:Release"  
        defines { "NDEBUG" }    
        optimize "On" 
        
        links
        {
            "glslang.lib",
            "MachineIndependent.lib",
            "GenericCodeGen.lib",
            "OGLCompiler.lib",
            "OSDependent.lib",
        }

        libdirs
        {
            "%{LibDirs.glslang_win}",
        }