project "OptickCore"
	uuid "830934D9-6F6C-C37D-18F2-FB3304348F00"
	kind "SharedLib"
	language "C++"
    cppdialect "C++14"
    configurations { "Debug", "Release" } 

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
    debugdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")

	folderDir = "../vendor/optick/"
	location "%{folderDir}"

	files 
	{
		"%{folderDir}" .. "src/**.cpp",
        "%{folderDir}" .. "src/**.h", 
	}

	includedirs
	{
		"%{folderDir}" .. "src"
	}

	defines 
	{ 
		"_CRT_SECURE_NO_WARNINGS",
		"OPTICK_EXPORTS",
		"USE_OPTICK=1",
		"OPTICK_ENABLE_GPU_D3D12=0",
		"OPTICK_ENABLE_GPU_VULKAN=0",
	}
	
	postbuildcommands
    {
       "{COPY} \"%{cfg.targetdir}/OptickCore.dll\" \"%{wks.location}/deps/".. outputdir..  "/dll/\"",
       "{COPY} \"%{cfg.targetdir}/OptickCore.lib\" \"%{wks.location}/deps/".. outputdir..  "/lib/\"",
    }

	filter "configurations:Debug"
		defines
		{ 
			 "_CRTDBG_MAP_ALLOC", 
			 "MT_INSTRUMENTED_BUILD",
		}

	filter "configurations:Release"
		optimize "Speed"
		defines
		{ 
			 "MT_INSTRUMENTED_BUILD",
		}

	filter "system:Windows"
		includedirs
		{
			"$(VULKAN_SDK)/Include",
		}
		libdirs 
		{
			"$(VULKAN_SDK)/Lib",
		}

		links 
		{ 
			"d3d12", 
			"dxgi",
			"vulkan-1",
		}
		defines 
		{ 
			"OPTICK_ENABLE_GPU_D3D12=1",
			"OPTICK_ENABLE_GPU_VULKAN=1",
			"NOMINMAX",
		}

    filter "system:linux"
		includedirs
		{
			"$(VULKAN_SDK)/Include",
		}
		libdirs 
		{
			"$(VULKAN_SDK)/Lib",
		}

		links 
		{ 
			"vulkan-1",
			"pthread",
		}
		defines 
		{ 
			"OPTICK_ENABLE_GPU_VULKAN=1"
		}