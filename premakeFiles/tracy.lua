project "tracy"
	kind "SharedLib"
	language "C++"
    cppdialect "C++14"
    configurations { "Debug", "Release" } 

    targetdir (outputdir_target .. "/%{prj.name}")
    objdir (outputdir_obj.. "/%{prj.name}")
    debugdir (outputdir_debug .. "/%{prj.name}")

	folderDirTracy = "../vendor/tracy/"
	location "../Engine/Vendor/tracy"

	files 
	{
        folderDirTracy.. "public/TracyClient.cpp", 
	}

	includedirs
	{
		folderDirTracy
	}

	defines 
	{ 
		"_CRT_SECURE_NO_WARNINGS",
		"TRACY_ENABLE",
		"TRACY_EXPORTS",
		"TRACY_ON_DEMAND",
		"TRACY_NO_CALLSTACK",
	}
	
	postbuildcommands
    {
       "{COPY} \"%{cfg.targetdir}/TracyClient.dll\" \"" .. output_deps .. "/dll/\"",
       "{COPY} \"%{cfg.targetdir}/TracyClient.lib\" \"" .. output_deps .. "/lib/\"",
       "{COPY} \"%{cfg.targetdir}/TracyClient.pdb\" \"" .. output_deps .. "/lib/\"",
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