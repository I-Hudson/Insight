local VULKAN_SDK = os.getenv("VULKAN_SDK")

project "ImGui"
	kind "StaticLib"
	language "C++"
	staticruntime "on"

    targetdir (outputdir_target .. "/%{prj.name}")
    objdir (outputdir_obj.. "/%{prj.name}")
    debugdir (outputdir_debug .. "/%{prj.name}")

	folderDirImGui = "../vendor/imgui/"
	folderDirImPlot = "../vendor/implot/"

	location "../Engine/Vendor/imgui"

	dependson
	{
		"glfw"
	}

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
		folderDirImGui .. "backends/imgui_impl_dx12.h",

		folderDirImGui .. "imgui.cpp",
		folderDirImGui .. "imgui_demo.cpp",
		folderDirImGui .. "imgui_draw.cpp",
		folderDirImGui .. "imgui_tables.cpp",
		folderDirImGui .. "imgui_widgets.cpp",
		folderDirImGui .. "misc/cpp/imgui_stdlib.cpp",
		folderDirImGui .. "backends/imgui_impl_glfw.cpp",
		folderDirImGui .. "backends/imgui_impl_win32.cpp",
		folderDirImGui .. "backends/imgui_impl_dx12.cpp",

		folderDirImPlot .. "implot.h",
		folderDirImPlot .. "implot_internal.h",
		folderDirImPlot .. "implot.cpp",
		folderDirImPlot .. "implot_items.cpp",
	}

	includedirs
	{
		folderDirImGui .. "./",
		"%{IncludeDirs.glfw}",
	}

    libdirs
    {
        "%{LibDirs.glfw}",
    }

	links 
	{
		"GLFW",
	}

	if VULKAN_SDK == nil then
	else
		files
		{
			folderDirImGui .. "backends/imgui_impl_vulkan.h",
			folderDirImGui .. "backends/imgui_impl_vulkan.cpp",
		}

		includedirs
		{
			"%{IncludeDirs.vulkan}",
		}

		libdirs
		{
			"%{LibDirs.vulkan}",
		}
	
		links 
		{
			"vulkan-1",
		}
	end

	postbuildcommands
    {
		"{COPY} \"%{cfg.targetdir}/ImGui.lib\" \"" .. output_deps .. "/lib/\"",
		"{COPY} \"%{cfg.targetdir}/ImGui.pdb\" \"" .. output_deps .. "/lib/\"",
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
