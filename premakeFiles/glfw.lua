project "GLFW"
	kind "StaticLib"
	language "C"
	
    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
    debugdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	
	folderDir = "../vendor/glfw/"
	location "%{folderDir}"

	files
	{
		"%{folderDir}" .. "include/GLFW/glfw3.h",
		"%{folderDir}" .. "include/GLFW/glfw3native.h",
		"%{folderDir}" .. "src/glfw_config.h",
		"%{folderDir}" .. "src/context.c",
		"%{folderDir}" .. "src/init.c",
		"%{folderDir}" .. "src/input.c",
		"%{folderDir}" .. "src/monitor.c",
		"%{folderDir}" .. "src/vulkan.c",
		"%{folderDir}" .. "src/window.c",
		"%{folderDir}" .. "src/platform.c",

		"%{folderDir}" .. "src/null_init.c",
		"%{folderDir}" .. "src/null_window.c",
		"%{folderDir}" .. "src/null_joystick.c",
		"%{folderDir}" .. "src/null_monitor.c",
	}

	filter "system:linux"
		pic "On"

		systemversion "latest"
		staticruntime "On"

		files
		{
			"%{folderDir}" .. "src/x11_init.c",
			"%{folderDir}" .. "src/x11_monitor.c",
			"%{folderDir}" .. "src/x11_window.c",
			"%{folderDir}" .. "src/xkb_unicode.c",
			"%{folderDir}" .. "src/posix_time.c",
			"%{folderDir}" .. "src/posix_thread.c",
			"%{folderDir}" .. "src/posix_module.c",
			"%{folderDir}" .. "src/glx_context.c",
			"%{folderDir}" .. "src/egl_context.c",
			"%{folderDir}" .. "src/osmesa_context.c",
			"%{folderDir}" .. "src/linux_joystick.c"
		}

		defines
		{
			"_GLFW_X11",
		}

	filter "system:windows"
		systemversion "latest"

		files
		{
			folderDir .. "src/win32_init.c",
			folderDir .. "src/win32_joystick.c",
			folderDir .. "src/win32_monitor.c",
			folderDir .. "src/win32_time.c",
			folderDir .. "src/win32_thread.c",
			folderDir .. "src/win32_window.c",
			folderDir .. "src/win32_module.c",
			folderDir .. "src/wgl_context.c",
			folderDir .. "src/egl_context.c",
			folderDir .. "src/osmesa_context.c"
		}

		defines 
		{ 
			"_GLFW_WIN32",
			"_CRT_SECURE_NO_WARNINGS"
		}

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"
