project "glm"  
    kind "StaticLib"   
    language "C++"
    cppdialect "C++17"
    configurations { "Debug", "Release" } 

    targetdir (outputdir_target .. "/%{prj.name}")
    objdir (outputdir_obj.. "/%{prj.name}")
    debugdir (outputdir_debug .. "/%{prj.name}")

    folderDir = "../vendor/glm/"
	location "%{folderDir}"
    
    defines
    {
    }
    
    includedirs
    {
        folderDir
    }

    files 
    { 
        folderDir .. "glm/**.hpp", 
        folderDir .. "glm/**.inl", 
        folderDir .. "glm/**.h", 
        folderDir .. "glm/**.cpp",
        folderDir .. "glm/**.c",
    }

    postbuildcommands
    {
       "{COPYFILE} \"%{cfg.targetdir}/glm.lib\" \"" .. output_deps .. "/lib/\"",
    }

    filter "configurations:Debug"
        defines { "DEBUG" }  
        symbols "On" 
    filter "configurations:Release"  
        defines { "NDEBUG" }    
        optimize "On" 