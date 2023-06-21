project "Reflect"  
    kind "SharedLib"   
    language "C++"
    cppdialect "C++17"
    configurations { "Debug", "Release" }
    
    targetdir (outputdir_target .. "/%{prj.name}")
    objdir (outputdir_obj.. "/%{prj.name}")
    debugdir (outputdir_debug .. "/%{prj.name}")

    folderDir = "../vendor/Reflect/Reflect/"
	location "../Engine/Vendor/Reflect"
    
    defines
    {
        "REFLECT_TYPE_INFO_ENABLED",
        "REFLECT_DLL_EXPORT",
    }

    files 
    {
        folderDir .. "/inc/**.hpp",
        folderDir .. "/inc/**.h",
        folderDir .. "/src/**.c",
        folderDir .. "/src/**.cpp",
    }

    includedirs 
    {
        folderDir .. "inc"
    }
    

     filter  "configurations:Debug"
        buildoptions "/MDd"
         runtime "Debug"
         symbols "on"
         targetname "Reflectd"
         postbuildcommands
         {
             "{COPY} \"%{cfg.targetdir}/Reflectd.lib\" \"" .. output_deps .. "/lib/\"",
             "{COPY} \"%{cfg.targetdir}/Reflectd.dll\" \"" .. output_deps .. "/dll/\"",
             "{COPY} \"%{cfg.targetdir}/Reflectd.pdb\" \"" .. output_deps .. "/lib/\"",
         }

     filter  "configurations:Release"
        buildoptions "/MD"
         runtime "Release"
         optimize "on"
         targetname "Reflect"
        postbuildcommands
        {
            "{COPY} \"%{cfg.targetdir}/Reflect.lib\" \"" .. output_deps .. "/lib/\"",
            "{COPY} \"%{cfg.targetdir}/Reflect.dll\" \"" .. output_deps .. "/dll/\"",
            "{COPY} \"%{cfg.targetdir}/Reflect.pdb\" \"" .. output_deps .. "/lib/\"",
        }