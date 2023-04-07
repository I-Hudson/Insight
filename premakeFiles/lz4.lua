project "lz4"  
    kind "StaticLib"   
    language "C++"
    cppdialect "C++17"
    configurations { "Debug", "Release" }
    
    targetdir (outputdir_target .. "/%{prj.name}")
    objdir (outputdir_obj.. "/%{prj.name}")
    debugdir (outputdir_debug .. "/%{prj.name}")

    folderDirLZ4 = "../vendor/lz4/"
	location "../Engine/Vendor/lz4"
    
    files 
    {
        folderDirLZ4 .. "lib/**.h",
        folderDirLZ4 .. "lib/**.c",
    }

    includedirs 
    {
        folderDirLZ4
    }

     filter  "configurations:Debug"
         runtime "Debug"
         symbols "on"
         targetname "lz4d"
         postbuildcommands
         {
             "{COPY} \"%{cfg.targetdir}/lz4d.lib\" \"" .. output_deps .. "/lib/\"",
             "{COPY} \"%{cfg.targetdir}/lz4d.pdb\" \"" .. output_deps .. "/lib/\"",
         }

     filter  "configurations:Release"
         runtime "Release"
         optimize "on"
         targetname "lz4"
        postbuildcommands
        {
            "{COPY} \"%{cfg.targetdir}/lz4.lib\" \"" .. output_deps .. "/lib/\"",
            "{COPY} \"%{cfg.targetdir}/lz4.pdb\" \"" .. output_deps .. "/lib/\"",
        }