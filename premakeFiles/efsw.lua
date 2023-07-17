newoption { trigger = "verbose", description = "Build efsw with verbose mode." }
newoption { trigger = "strip-symbols", description = "Strip debugging symbols in other file ( only for relwithdbginfo configuration )." }
newoption { trigger = "legacy", description = "Build efsw with cpp legacy support (before C++11)." }
newoption { trigger = "thread-sanitizer", description ="Compile with ThreadSanitizer" }

efsw_major_version	= "1"
efsw_minor_version	= "0"
efsw_patch_version	= "2"
efsw_version		= efsw_major_version .. "." .. efsw_minor_version .. "." .. efsw_patch_version

function get_include_paths()
	local function _insert_include_paths( file )
		local function _trim(s)
			return (s:gsub("^%s*(.-)%s*$", "%1"))
		end

		local paths = { }
		local lines = file:read('*all')

		for line in string.gmatch(lines, '([^\n]+)')
		do
			table.insert( paths, _trim( line ) )
		end

		file:close()

		return paths
	end

	local file = io.popen( "echo | gcc -Wp,-v -x c++ - -fsyntax-only 2>&1 | grep -v '#' | grep '/'", 'r' )
	local include_paths = _insert_include_paths( file )

	if next(include_paths) == nil then
		file = io.popen( "echo | clang++ -Wp,-v -x c++ - -fsyntax-only 2>&1 | grep -v '#' | grep '/' | grep -v 'nonexistent'", 'r' )

		include_paths = _insert_include_paths( file )

		if next(include_paths) == nil then
			table.insert( include_paths, "/usr/include" )
			table.insert( include_paths, "/usr/local/include" )
		end
	end

	return include_paths
end

function inotify_header_exists()
	local efsw_include_paths = get_include_paths()

	for _,v in pairs( efsw_include_paths )
	do
		local cur_path = v .. "/sys/inotify.h"

		if os.isfile( cur_path ) then
			return true
		end
	end

	return false
end

function string.starts(String,Start)
	if ( _ACTION ) then
		return string.sub(String,1,string.len(Start))==Start
	end

	return false
end

function is_vs()
	return ( string.starts(_ACTION,"vs") )
end

function conf_warnings()
	if not is_vs() then
		buildoptions{ "-Wall -Wno-long-long" }

		if not os.istarget("windows") then
			buildoptions{ "-fPIC" }
		end
	else
		defines { "_SCL_SECURE_NO_WARNINGS" }
	end

	if _OPTIONS["thread-sanitizer"] then
		buildoptions { "-fsanitize=thread" }
		linkoptions { "-fsanitize=thread" }
		if not os.istarget("macosx") then
			links { "tsan" }
		end
	end
end

function conf_links()
	if not os.istarget("windows") and not os.istarget("haiku") then
		links { "pthread" }
	end

	if os.istarget("macosx") then
		links { "CoreFoundation.framework", "CoreServices.framework" }
	end
end

function conf_excludes()
	if os.istarget("windows") then
		excludes { "src/efsw/WatcherKqueue.cpp", "src/efsw/WatcherFSEvents.cpp", "src/efsw/WatcherInotify.cpp", "src/efsw/FileWatcherKqueue.cpp", "src/efsw/FileWatcherInotify.cpp", "src/efsw/FileWatcherFSEvents.cpp" }
	elseif os.istarget("linux") then
		excludes { "src/efsw/WatcherKqueue.cpp", "src/efsw/WatcherFSEvents.cpp", "src/efsw/WatcherWin32.cpp", "src/efsw/FileWatcherKqueue.cpp", "src/efsw/FileWatcherWin32.cpp", "src/efsw/FileWatcherFSEvents.cpp" }
	elseif os.istarget("macosx") then
		excludes { "src/efsw/WatcherInotify.cpp", "src/efsw/WatcherWin32.cpp", "src/efsw/FileWatcherInotify.cpp", "src/efsw/FileWatcherWin32.cpp" }
	elseif os.istarget("bsd") then
		excludes { "src/efsw/WatcherInotify.cpp", "src/efsw/WatcherWin32.cpp", "src/efsw/WatcherFSEvents.cpp", "src/efsw/FileWatcherInotify.cpp", "src/efsw/FileWatcherWin32.cpp", "src/efsw/FileWatcherFSEvents.cpp" }
	end

	if os.istarget("linux") and not inotify_header_exists() then
		defines { "EFSW_INOTIFY_NOSYS" }
	end
end

if os.istarget("windows") then
	osfiles = "src/efsw/platform/win/*.cpp"
else
	osfiles = "src/efsw/platform/posix/*.cpp"
end

project "efsw"
	kind "StaticLib"   
	language "C++"
	cppdialect "C++17"
	configurations { "Debug", "Release" } 

	targetdir (outputdir_target .. "/%{prj.name}")
    objdir (outputdir_obj.. "/%{prj.name}")
    debugdir (outputdir_debug .. "/%{prj.name}")

	conf_excludes()

	folderDir = "../vendor/efsw/"
	location "../Engine/Vendor/efsw"

	includedirs 
	{ 
		folderDir .. "include", 
		folderDir .. "src",
	}
	files 
	{ 
		folderDir .. "/src/efsw/*.cpp",
		folderDir .. "/" .. osfiles,
	}

	filter "configurations:Debug"
		defines { "DEBUG" }
		symbols "On"
		targetname "efswd"
		conf_warnings()
		postbuildcommands
		{
			"{COPY} \"%{cfg.targetdir}/efswd.lib\" \"" .. output_deps .. "/lib/\"",
			"{COPY} \"%{cfg.targetdir}/efswd.pdb\" \"" .. output_deps .. "/lib/\"",
		}

	filter "configurations:Release"
		defines { "NDEBUG" }
		optimize "On"
		targetname "efsw"
		conf_warnings()
		postbuildcommands
		{
			"{COPY} \"%{cfg.targetdir}/efsw.lib\" \"" .. output_deps .. "/lib/\"",
			"{COPY} \"%{cfg.targetdir}/efsw.pdb\" \"" .. output_deps .. "/lib/\"",
		}
