project "tracy"
	kind "SharedLib"
	language "C++"
    cppdialect "C++14"
    configurations { "Debug", "Release" } 

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
    debugdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")

	folderDirTracy = "../vendor/tracy/"
	location "%{folderDirTracy}"

	files 
	{
        folderDirTracy.. "TracyClient.cpp", 
	}

	includedirs
	{
		folderDirTracy
	}

	defines 
	{ 
		"_CRT_SECURE_NO_WARNINGS",
		"TRACY_ENABLE",
		"TRACY_EXPORTS"
	}
	
	postbuildcommands
    {
       "{COPY} \"%{cfg.targetdir}/tracy.dll\" \"%{wks.location}/deps/".. outputdir..  "/dll/\"",
       "{COPY} \"%{cfg.targetdir}/tracy.lib\" \"%{wks.location}/deps/".. outputdir..  "/lib/\"",
    }

	filter "configurations:Debug"
		defines
		{ 
		}

	filter "configurations:Release"
		optimize "Speed"
		defines
		{ 
		}