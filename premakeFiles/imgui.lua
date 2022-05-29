project "ImGui"
	kind "StaticLib"
	language "C++"
	staticruntime "on"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
    debugdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")

	folderDir = "../vendor/imgui/"
	location "%{folderDir}"

	files
	{
        "%{folderDir}" .. "imgui.h",
        "%{folderDir}" .. "imconfig.h",
        "%{folderDir}" .. "imgui_internal.h",
        "%{folderDir}" .. "imstb_rectpack.h",
        "%{folderDir}" .. "imstb_textedit.h",
        "%{folderDir}" .. "imstb_truetype.h",
		"%{folderDir}" .. "misc/cpp/imgui_stdlib.h",
		"%{folderDir}" .. "backends/imgui_impl_glfw.h",
		"%{folderDir}" .. "backends/imgui_impl_win32.h",
		"%{folderDir}" .. "backends/imgui_impl_vulkan.h",
		"%{folderDir}" .. "backends/imgui_impl_dx12.h",

		"%{folderDir}" .. "imgui.cpp",
		"%{folderDir}" .. "imgui_demo.cpp",
		"%{folderDir}" .. "imgui_draw.cpp",
		"%{folderDir}" .. "imgui_tables.cpp",
		"%{folderDir}" .. "imgui_widgets.cpp",
		"%{folderDir}" .. "misc/cpp/imgui_stdlib.cpp",
		"%{folderDir}" .. "backends/imgui_impl_glfw.cpp",
		"%{folderDir}" .. "backends/imgui_impl_win32.cpp",
		"%{folderDir}" .. "backends/imgui_impl_vulkan.cpp",
		"%{folderDir}" .. "backends/imgui_impl_dx12.cpp",
	}

	defines
	{
	}

	includedirs
	{
		"%{folderDir}" .. "./",
		"%{folderDir}" .. "%{IncludeDirs.glfw}",
		"%{folderDir}" .. "%{IncludeDirs.vulkan}",
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
