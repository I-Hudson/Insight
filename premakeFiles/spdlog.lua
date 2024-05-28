project "spdlog"
	kind "StaticLib"
	language "C++"
    configurations { "Debug", "Release" } 
	buildoptions "/MDd"

    targetdir (outputdir_target .. "/%{prj.name}")
    objdir (outputdir_obj.. "/%{prj.name}")
    debugdir (outputdir_debug .. "/%{prj.name}")

	folderDirSpdlog = "../vendor/spdlog/"
	location "../Engine/Vendor/spdlog"

	files 
	{
        folderDirSpdlog.. "src/**.cpp",
        folderDirSpdlog.. "include/**.h",
	}

	includedirs
	{
		folderDirSpdlog .. "include",
	}

	defines 
	{ 
		"_CRT_SECURE_NO_WARNINGS",
		"FMT_EXPORT",
		"FMT_SHARED",
		"SPDLOG_COMPILED_LIB",
		--"SPDLOG_SHARED_LIB",
		--"spdlog_EXPORTS",
	}
	
	postbuildcommands
    {
       "{COPY} \"%{cfg.targetdir}/spdlog.dll\" \"" .. output_deps .. "/dll/\"",
       "{COPY} \"%{cfg.targetdir}/spdlog.lib\" \"" .. output_deps .. "/lib/\"",
       "{COPY} \"%{cfg.targetdir}/spdlog.pdb\" \"" .. output_deps .. "/lib/\"",
    }

	filter "configurations:Debug"
		buildoptions "/MDd"

	filter "configurations:Release"
		optimize "Speed"