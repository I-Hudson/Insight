project "Splash"  
    kind "StaticLib"   
    language "C++"
    cppdialect "C++17"
    configurations { "Debug", "Release" }
    
    targetdir (outputdir_target .. "/%{prj.name}")
    objdir (outputdir_obj.. "/%{prj.name}")
    debugdir (outputdir_debug .. "/%{prj.name}")

    folderDirAssimp = "../vendor/Splash/Splash/"
	location "../Engine/Vendor/Splash"
    
    files 
    {
        folderDirAssimp .. "**.cpp",
        folderDirAssimp .. "**.hpp",
        folderDirAssimp .. "**.c",
        folderDirAssimp .. "**.h",
    }

    includedirs 
    {
        folderDirAssimp .. "inc"
    }

    if os.host() == "windows" then
        defines { "SPLASH_PLATFORM_WINDOWS" }
    end
    

     filter  "configurations:Debug"
         runtime "Debug"
         symbols "on"
         targetname "Splashd"
         postbuildcommands
         {
             "{COPY} \"%{cfg.targetdir}/Splashd.lib\" \"" .. output_deps .. "/lib/\"",
             "{COPY} \"%{cfg.targetdir}/Splashd.pdb\" \"" .. output_deps .. "/lib/\"",
         }

     filter  "configurations:Release"
         runtime "Release"
         optimize "on"
         targetname "Splash"
        postbuildcommands
        {
            "{COPY} \"%{cfg.targetdir}/Splash.lib\" \"" .. output_deps .. "/lib/\"",
            "{COPY} \"%{cfg.targetdir}/Splash.pdb\" \"" .. output_deps .. "/lib/\"",
        }