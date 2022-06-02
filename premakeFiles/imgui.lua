project "ImGui"
	kind "StaticLib"
	language "C++"
	staticruntime "on"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
    debugdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")

	folderDirImGui = "../vendor/imgui/"
	location "%{folderDirImGui}"

	files
	{
        folderDirImGui .. "imgui.h",
        folderDirImGui .. "imconfig.h",
        folderDirImGui .. "imgui_internal.h",
        folderDirImGui .. "imstb_rectpack.h",
        folderDirImGui .. "imstb_textedit.h",
        folderDirImGui .. "imstb_truetype.h",
		folderDirImGui .. "misc/cpp/imgui_stdlib.h",
		folderDirImGui .. "backends/imgui_impl_glfw.h",
		folderDirImGui .. "backends/imgui_impl_win32.h",
		folderDirImGui .. "backends/imgui_impl_vulkan.h",
		folderDirImGui .. "backends/imgui_impl_dx12.h",

		folderDirImGui .. "imgui.cpp",
		folderDirImGui .. "imgui_demo.cpp",
		folderDirImGui .. "imgui_draw.cpp",
		folderDirImGui .. "imgui_tables.cpp",
		folderDirImGui .. "imgui_widgets.cpp",
		folderDirImGui .. "misc/cpp/imgui_stdlib.cpp",
		folderDirImGui .. "backends/imgui_impl_glfw.cpp",
		folderDirImGui .. "backends/imgui_impl_win32.cpp",
		folderDirImGui .. "backends/imgui_impl_vulkan.cpp",
		folderDirImGui .. "backends/imgui_impl_dx12.cpp",
	}

	defines
	{
	}

	includedirs
	{
		folderDirImGui .. "./",
		folderDirImGui .. "%{IncludeDirs.glfw}",
		folderDirImGui .. "%{IncludeDirs.vulkan}",
	}

    libdirs
    {
        "%{wks.location}/vendor/glfw/lib",
        "%{LibDirs.vulkan}",
    }

	links 
	{
		"GLFW",
		"vulkan-1",
	}

	postbuildcommands
    {
		"{COPYFILE} \"%{cfg.targetdir}/ImGui.lib\" \"%{wks.location}/deps/".. outputdir..  "/lib/\"",
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
