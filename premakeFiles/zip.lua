project "zip"
	kind "StaticLib"
	language "C++"
	staticruntime "on"

    targetdir (outputdir_target .. "/%{prj.name}")
    objdir (outputdir_obj.. "/%{prj.name}")
    debugdir (outputdir_debug .. "/%{prj.name}")

	folderDirZip = "../vendor/zip/"

	location "../Engine/Vendor/zip"

	dependson
	{
	}

	files
	{
        folderDirZip .. "src/miniz.h",
        folderDirZip .. "src/zip.h",
        folderDirZip .. "src/zip.c",
	}

	includedirs
	{
		folderDirZip .. "/src",
	}

    libdirs
    {
    }

	links 
	{
	}

	postbuildcommands
    {
		"{COPY} \"%{cfg.targetdir}/zip.lib\" \"" .. output_deps .. "/lib/\"",
		"{COPY} \"%{cfg.targetdir}/zip.pdb\" \"" .. output_deps .. "/lib/\"",
    }

	filter "system:linux"
		pic "On"

		systemversion "latest"
		staticruntime "On"

	filter "system:windows"
		systemversion "latest"
		staticruntime "On"

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"
        buildoptions "/MDd"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"
        buildoptions "/MD"
