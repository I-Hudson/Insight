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
        "%{IncludeDirs.imgui}",
        "%{IncludeDirs.imgui}",
        "%{IncludeDirs.spdlog}",
        "%{IncludeDirs.vma}",
        "%{IncludeDirs.glslang}win_debug/inc",
        "%{IncludeDirs.vulkan}",
        "%{IncludeDirs.spirv_reflect}",
        "%{IncludeDirs.dxcompiler}",
        "%{IncludeDirs.assimp}",
        "%{IncludeDirs.assimp}/../build/include",
        "%{IncludeDirs.optick}",
        "%{IncludeDirs.stb_image}",
    }

    files 
    { 
        "inc/**.hpp", 
        "inc/**.h", 
        "src/**.cpp",

        "../vendor/SPIRV-Reflect/spirv_reflect.h",
        "../vendor/SPIRV-Reflect/spirv_reflect.cpp",
        "../vendor/stb/stb_image.h",
        "../vendor/stb/stb_image_write.h",
    }

    links
    {
        "InsightCore",
        "glfw3.lib",
        "glm",
        "imgui",
        "vulkan-1.lib",
        "D3d12.lib",
        "DXGI.lib",
        "dxguid.lib",
        "dxcompiler.lib",
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

    filter "configurations:Debug or configurations:Testing"
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
            "assimp-vc143-mtd.lib",
            "Optickcore.lib",
        }

        libdirs
        {
            "%{LibDirs.glslang_win_d}",
            "%{LibDirs.dxcompiler_win_d}",
            "%{wks.location}/deps/lib/debug",
        }

        postbuildcommands
        {
            "{COPY} \"%{wks.location}/vendor/dxcompiler/win_debug/bin/x64/dxcompiler.dll\" \"%{wks.location}/bin/".. outputdir..  "/InsightEditor/\"",
            "{COPY} \"%{wks.location}/vendor/dxcompiler/win_debug/bin/x64/dxil.dll\" \"%{wks.location}/bin/".. outputdir..  "/InsightEditor/\"",
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
            "assimp-vc143-mt.lib",
            "Optickcore.lib",
        }

        libdirs
        {
            "%{LibDirs.glslang_win}",
            "%{LibDirs.dxcompiler_win}",
            "%{wks.location}/deps/lib/release",
        }

        postbuildcommands
        {
            "{COPY} \"%{wks.location}/vendor/dxcompiler/win_release/bin/x64/dxcompiler.dll\" \"%{wks.location}/bin/".. outputdir..  "/InsightEditor/\"",
            "{COPY} \"%{wks.location}/vendor/dxcompiler/win_release/bin/x64/dxil.dll\" \"%{wks.location}/bin/".. outputdir..  "/InsightEditor/\"",
        }