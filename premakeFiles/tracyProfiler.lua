

local CommonConfig = require "../Engine/lua/CommonConfig"
local folderDirTracy = "../vendor/tracy/"

local capstoneRepoDir = folderDirTracy .. "profiler/deps/capstone_src"
local capstoneGithubUrl = "https://github.com/capstone-engine/capstone"
local capstoneGitTag = "5.0.3"
if os.rename(capstoneRepoDir, capstoneRepoDir) == nil then
	print(capstoneRepoDir .. "repo didn't exist. Cloning it now.")
	CommonConfig.OpenCMD("git clone --branch " .. capstoneGitTag .. " " .. capstoneGithubUrl .. " " ..  capstoneRepoDir)
end

local glfwRepoDir = folderDirTracy .. "profiler/deps/glfw_src"
local glfwGithubUrl = "https://github.com/glfw/glfw"
local glfwGitTag = "3.4"
if os.rename(glfwRepoDir, glfwRepoDir) == nil then
	CommonConfig.GitClone(glfwGitTag, glfwGithubUrl, glfwRepoDir)
end

local freetypeRepoDir = folderDirTracy .. "profiler/deps/freetype_src"
local freetypeGithubUrl = "https://github.com/freetype/freetype"
local freetypeGitTag = "VER-2-13-2"
if os.rename(freetypeRepoDir, freetypeRepoDir) == nil then
	CommonConfig.GitClone(freetypeGitTag, freetypeGithubUrl, freetypeRepoDir)

end

local oneTBBRepoDir = folderDirTracy .. "profiler/deps/freetype_src"
local oneTBBGithubUrl ="https://github.com/oneapi-src/oneTBB"
local oneTBBGitTag = "v2021.12.0-rc2"
if os.rename(oneTBBRepoDir, oneTBBRepoDir) == nil then
	CommonConfig.GitClone(oneTBBGitTag, oneTBBGithubUrl, oneTBBRepoDir)

end

project "tracyProfiler"
	kind "WindowedApp"
	language "C++"
    cppdialect "C++20"
    configurations { "Debug", "Release" } 

    targetdir (outputdir_target .. "/%{prj.name}")
    objdir (outputdir_obj.. "/%{prj.name}")
    debugdir (outputdir_debug .. "/%{prj.name}")

	location "../Engine/Vendor/tracy"

	files 
	{
		folderDirTracy .. "profiler/deps/capstone_src/arch/**.hpp",
		folderDirTracy .. "profiler/deps/capstone_src/arch/**.h",
		folderDirTracy .. "profiler/deps/capstone_src/arch/**.cpp",
		folderDirTracy .. "profiler/deps/capstone_src/arch/**.c",

		folderDirTracy .. "profiler/deps/freetype_src/include/**.hpp",
		folderDirTracy .. "profiler/deps/freetype_src/include/**.h",
		folderDirTracy .. "profiler/deps/freetype_src/include/**.cpp",
		folderDirTracy .. "profiler/deps/freetype_src/include/**.c",

		folderDirTracy .. "profiler/deps/freetype_src/src/**.hpp",
		folderDirTracy .. "profiler/deps/freetype_src/src/**.h",
		folderDirTracy .. "profiler/deps/freetype_src/src/**.cpp",
		folderDirTracy .. "profiler/deps/freetype_src/src/**.c",

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

		folderDirTracy.. "profiler/deps/capstone_src",
		folderDirTracy.. "profiler/deps/capstone_src/include",
		folderDirTracy.. "profiler/deps/freetype_src",
		folderDirTracy.. "profiler/deps/freetype_src/include",
		folderDirTracy.. "profiler/deps/glfw_src/include",

		folderDirTracy.. "zstd",

		folderDirTracy.. "vcpkg_installed/x64-windows-static/include/brotli",
		folderDirTracy.. "profiler/build/_deps/capstone-src/include/capstone",
		folderDirTracy.. "rofiler/build/_deps/freetype-build/include",
		folderDirTracy.. "profiler/build/_deps/glfw-src/include",
		folderDirTracy.. "vcpkg_installed/x64-windows-static/include/libpng16",
		folderDirTracy.. "vcpkg_installed/x64-windows-static/include",
	}

	links
	{
		--"capstone.lib",
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

		"CAPSTONE_ARM_SUPPORT",
		"CAPSTONE_ARM64_SUPPORT",
		"CAPSTONE_M68K_SUPPORT",
		"CAPSTONE_MIPS_SUPPORT",
		"CAPSTONE_PPC_SUPPORT",
		"CAPSTONE_SPARC_SUPPORT",
		"CAPSTONE_SYSZ_SUPPORT",
		"CAPSTONE_XCORE_SUPPORT",
		"CAPSTONE_X86_SUPPORT",
		"CAPSTONE_TMS320C64X_SUPPORT",
		"CAPSTONE_M680X_SUPPORT",
		"CAPSTONE_EVM_SUPPORT",
		"CAPSTONE_MOS65XX_SUPPORT",
		"CAPSTONE_WASM_SUPPORT",
		"CAPSTONE_BPF_SUPPORT",
		"CAPSTONE_RISCV_SUPPORT",
		"CAPSTONE_SH_SUPPORT",
		"CAPSTONE_TRICORE_SUPPORT",
		"CAPSTONE_USE_SYS_DYN_MEM",
		"CAPSTONE_HAS_ARM",
		"CAPSTONE_HAS_ARM64",
		"CAPSTONE_HAS_MIPS",
		"CAPSTONE_HAS_POWERPC",
		"CAPSTONE_HAS_X86",
		"CAPSTONE_HAS_SPARC",
		"CAPSTONE_HAS_SYSZ",
		"CAPSTONE_HAS_XCORE",
		"CAPSTONE_HAS_M68K",
		"CAPSTONE_HAS_TMS320C64X",
		"CAPSTONE_HAS_M680X",
		"CAPSTONE_HAS_EVM",
		"CAPSTONE_HAS_WASM",
		"CAPSTONE_HAS_MOS65XX",
		"CAPSTONE_HAS_BPF",
		"CAPSTONE_HAS_RISCV",
		"CAPSTONE_HAS_SH",
		"CAPSTONE_HAS_TRICORE",
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