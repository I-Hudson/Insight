project "meshoptimizer"
	kind "StaticLib"
	language "C++"
    cppdialect "C++14"
    configurations { "Debug", "Release" } 
	targetname  "meshoptimizer"

    targetdir (outputdir_target .. "/%{prj.name}")
    objdir (outputdir_obj.. "/%{prj.name}")
    debugdir (outputdir_debug .. "/%{prj.name}")

	folderDirMeshOptimizer = "../vendor/meshoptimizer/"
	location "%{folderDirMeshOptimizer}"

	files 
	{
        folderDirMeshOptimizer.. "src/**.h",
        folderDirMeshOptimizer.. "src/**.cpp", 
	}

	includedirs
	{
		folderDirMeshOptimizer .. "src"
	}

	defines 
	{ 
		"_CRT_SECURE_NO_WARNINGS",
	}
	
	postbuildcommands
    {
       "{COPY} \"%{cfg.targetdir}/meshoptimizer.lib\" \"" .. output_deps .. "/lib/\"",
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