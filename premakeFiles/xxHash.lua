project "xxHash"  
    kind "StaticLib"   
    language "C++"
    cppdialect "C++17"
    configurations { "Debug", "Release" }
    
    targetdir (outputdir_target .. "/%{prj.name}")
    objdir (outputdir_obj.. "/%{prj.name}")
    debugdir (outputdir_debug .. "/%{prj.name}")

    folderDirAssimp = "../vendor/xxHash/"
	location "../Engine/Vendor/xxHash"
    
    files 
    {
        folderDirAssimp .. "xxhash.h",
        folderDirAssimp .. "xxhash.c",
    }

    includedirs 
    {
        folderDirAssimp
    }

     filter  "configurations:Debug"
         runtime "Debug"
         symbols "on"
         targetname "xxHashd"
         postbuildcommands
         {
             "{COPY} \"%{cfg.targetdir}/xxHashd.lib\" \"" .. output_deps .. "/lib/\"",
             "{COPY} \"%{cfg.targetdir}/xxHashd.pdb\" \"" .. output_deps .. "/lib/\"",
         }

     filter  "configurations:Release"
         runtime "Release"
         optimize "on"
         targetname "xxHash"
        postbuildcommands
        {
            "{COPY} \"%{cfg.targetdir}/xxHash.lib\" \"" .. output_deps .. "/lib/\"",
            "{COPY} \"%{cfg.targetdir}/xxHash.pdb\" \"" .. output_deps .. "/lib/\"",
        }