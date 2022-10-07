project "doctest"
	kind "SharedLib"
	language "C++"
    cppdialect "C++14"
    configurations { "Testing" } 

    targetdir (outputdir_target .. "/%{prj.name}")
    objdir (outputdir_obj .. "/%{prj.name}")
    debugdir (outputdir_debug .. "/%{prj.name}")

	folderDirDoctest = "../vendor/doctest/"
	location "../Engine/Vendor/doctest"

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
		"{COPY} \"%{cfg.targetdir}/doctest.dll\" \"" .. output_deps .. "/dll/\"",
		"{COPY} \"%{cfg.targetdir}/doctest.lib\" \"" .. output_deps .. "/lib/\"",
    }