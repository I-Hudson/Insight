project "GLFW"
	kind "StaticLib"
	language "C"
	
    targetdir (outputdir_target .. "/%{prj.name}")
    objdir (outputdir_obj.. "/%{prj.name}")
    debugdir (outputdir_debug .. "/%{prj.name}")
	
	folderDirGLFW = "../vendor/glfw/"
	location "../Engine/Vendor/glfw"

	files
	{
		folderDirGLFW .. "include/GLFW/glfw3.h",
		folderDirGLFW .. "include/GLFW/glfw3native.h",
		folderDirGLFW .. "src/glfw_config.h",
		folderDirGLFW .. "src/context.c",
		folderDirGLFW .. "src/init.c",
		folderDirGLFW .. "src/input.c",
		folderDirGLFW .. "src/monitor.c",
		folderDirGLFW .. "src/vulkan.c",
		folderDirGLFW .. "src/window.c",
		folderDirGLFW .. "src/platform.c",

		folderDirGLFW .. "src/null_init.c",
		folderDirGLFW .. "src/null_window.c",
		folderDirGLFW .. "src/null_joystick.c",
		folderDirGLFW .. "src/null_monitor.c",
	}

	filter "system:linux"
		pic "On"

		systemversion "latest"
		staticruntime "On"

		files
		{
			folderDirGLFW .. "src/x11_init.c",
			folderDirGLFW .. "src/x11_monitor.c",
			folderDirGLFW .. "src/x11_window.c",
			folderDirGLFW .. "src/xkb_unicode.c",
			folderDirGLFW .. "src/posix_time.c",
			folderDirGLFW .. "src/posix_thread.c",
			folderDirGLFW .. "src/posix_module.c",
			folderDirGLFW .. "src/glx_context.c",
			folderDirGLFW .. "src/egl_context.c",
			folderDirGLFW .. "src/osmesa_context.c",
			folderDirGLFW .. "src/linux_joystick.c"
		}

		defines
		{
			"_GLFW_X11",
		}

	filter "system:windows"
		systemversion "latest"

		files
		{
			folderDirGLFW .. "src/win32_init.c",
			folderDirGLFW .. "src/win32_joystick.c",
			folderDirGLFW .. "src/win32_monitor.c",
			folderDirGLFW .. "src/win32_time.c",
			folderDirGLFW .. "src/win32_thread.c",
			folderDirGLFW .. "src/win32_window.c",
			folderDirGLFW .. "src/win32_module.c",
			folderDirGLFW .. "src/wgl_context.c",
			folderDirGLFW .. "src/egl_context.c",
			folderDirGLFW .. "src/osmesa_context.c"
		}

		defines 
		{ 
			"_GLFW_WIN32",
			"_CRT_SECURE_NO_WARNINGS"
		}

		postbuildcommands
		{
			"{COPY} \"%{cfg.targetdir}/GLFW.lib\" \"" .. output_deps .. "/lib/\"",
			"{COPY} \"%{cfg.targetdir}/GLFW.pdb\" \"" .. output_deps .. "/lib/\"",
		}

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"
