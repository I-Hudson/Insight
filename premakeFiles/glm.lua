project "glm"  
    kind "StaticLib"   
    language "C++"
    cppdialect "C++17"
    configurations { "Debug", "Release" } 

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
    debugdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")

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
       "{COPYFILE} \"%{cfg.targetdir}/glm.lib\" \"%{wks.location}/deps/".. outputdir..  "/lib/\"",
    }

    filter "configurations:Debug"
        defines { "DEBUG" }  
        symbols "On" 
    filter "configurations:Release"  
        defines { "NDEBUG" }    
        optimize "On" 