project "tracyProfiler"
	kind "WindowedApp"
	language "C++"
    cppdialect "C++20"
    configurations { "Debug", "Release" } 

    targetdir (outputdir_target .. "/%{prj.name}")
    objdir (outputdir_obj.. "/%{prj.name}")
    debugdir (outputdir_debug .. "/%{prj.name}")

	folderDirTracy = "../vendor/tracy/"
	location "../Engine/Vendor/tracy"

	files 
	{
        folderDirTracy.. "public/common/**.hpp", 
        folderDirTracy.. "public/common/**.h", 
        folderDirTracy.. "public/common/**.cpp", 
        folderDirTracy.. "public/common/**.c", 

        folderDirTracy.. "imgui/**.h", 
        folderDirTracy.. "imgui/**.cpp",
        folderDirTracy.. "imgui/**.c",

		folderDirTracy.. "nfd/nfd.h", 
        folderDirTracy.. "nfd/nfd_win.cpp",

        folderDirTracy.. "server/**.hpp",
        folderDirTracy.. "server/**.h",
        folderDirTracy.. "server/**.cpp",
        folderDirTracy.. "server/**.c",

		folderDirTracy.. "profiler/src/**.hpp",
		folderDirTracy.. "profiler/src/**.h",
        folderDirTracy.. "profiler/src/**.cpp",
        folderDirTracy.. "profiler/src/**.c",

        folderDirTracy.. "zstd/**.hpp",
        folderDirTracy.. "zstd/**.h",
        folderDirTracy.. "zstd/**.cpp",
        folderDirTracy.. "zstd/**.c",
	}

	includedirs
	{
		folderDirTracy,
		folderDirTracy.. "public",
		folderDirTracy.. "imgui",
		folderDirTracy.. "nfd",
		folderDirTracy.. "server",
		folderDirTracy.. "profiler/src",
		folderDirTracy.. "zstd",

		folderDirTracy.. "vcpkg_installed/x64-windows-static/include/brotli",
		folderDirTracy.. "profiler/build/_deps/capstone-src/include/capstone",
		folderDirTracy.. "profiler/build/_deps/capstone-src/include",
		folderDirTracy.. "rofiler/build/_deps/freetype-build/include",
		folderDirTracy.. "profiler/build/_deps/freetype-src/include",
		folderDirTracy.. "profiler/build/_deps/glfw-src/include",
		folderDirTracy.. "vcpkg_installed/x64-windows-static/include/libpng16",
		folderDirTracy.. "vcpkg_installed/x64-windows-static/include",
	}

	links
	{
		"capstone.lib",
		"glfw3.lib",

		"ws2_32.lib",
		"Dbghelp.lib",
	}

	defines 
	{ 
		"NDEBUG",
		"_CRT_SECURE_NO_DEPRECATE",
		"_CRT_NONSTDC_NO_DEPRECATE",
		"WIN32_LEAN_AND_MEAN",
		"NOMINMAX",
		"_USE_MATH_DEFINES",
		"IMGUI_ENABLE_FREETYPE",
	}
	removedefines "TRACY_IMPORTS"

	filter "configurations:Debug"
		libdirs
		{
			folderDirTracy .. "vcpkg_installed/x64-windows-static/debug/lib"
		}
		links
		{
			"bz2d.lib",
			"freetyped.lib",
			"libpng16d.lib",
			"zlibd.lib",
		}

		defines
		{ 
		}

	filter "configurations:Release"
		optimize "Speed"
		libdirs
		{
			folderDirTracy.. "vcpkg_installed/x64-windows-static/lib"
		}
		links
		{
			"bz2.lib",
			"freetype.lib",
			"libpng16.lib",
			"zlib.lib",
		}
		defines
		{ 
		}
		
    filter "system:Windows"
		files
		{
			folderDirTracy.. "profiler/win32/Tracy.manifest",
			folderDirTracy.. "profiler/win32/Tracy.rc",
		}
		removefiles 
		{
			folderDirTracy.. "profiler/src/wayland/**.c",
			folderDirTracy.. "profiler/src/wayland/**.h",
			folderDirTracy.. "profiler/src/BackendWayland.cpp",
		}