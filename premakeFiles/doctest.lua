project "doctest"
	kind "SharedLib"
	language "C++"
    cppdialect "C++14"
    configurations { "Testing" } 

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
    debugdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")

	folderDirDoctest = "../vendor/doctest/"
	location "%{folderDirDoctest}"

	files 
	{
		folderDirDoctest.. "main.cpp",
        folderDirDoctest.. "doctest/doctest.h", 
	}

	includedirs
	{
		folderDirDoctest .. "src"
	}

	defines 
	{ 
		"_CRT_SECURE_NO_WARNINGS",
		"DOCTEST_CONFIG_IMPLEMENTATION_IN_DLL",
	}
	
	postbuildcommands
    {
		"{COPYFILE} \"%{cfg.targetdir}/doctest.dll\" \"%{wks.location}/deps/".. outputdir..  "/dll/\"",
		"{COPYFILE} \"%{cfg.targetdir}/doctest.lib\" \"%{wks.location}/deps/".. outputdir..  "/lib/\"",
    }