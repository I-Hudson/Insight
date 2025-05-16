project "nvrhi"  
    kind "StaticLib"   
    language "C++"
    cppdialect "C++17"
    configurations { "Debug", "Release" }
    
    targetdir (outputdir_target .. "/%{prj.name}")
    objdir (outputdir_obj.. "/%{prj.name}")
    debugdir (outputdir_debug .. "/%{prj.name}")

    folderDir = "../vendor/NVRHI/"
	location "../Engine/Vendor/NVRHI"
    
    files 
    {
        folderDir .. "include/**.h",
        folderDir .. "include/**.cpp",

        folderDir .. "src/**.h",
        folderDir .. "src/**.cpp",

        folderDir .. "tools/nvrhi.natvis",
    }

    includedirs 
    {
        folderDir .. "include",
        folderDir .. "/thirdparty/DirectX-Headers/include",
        folderDir .. "/thirdparty/Vulkan-Headers/include",
    }

    postbuildcommands
    {
        "{COPY} \"%{cfg.targetdir}/nvrhi.lib\" \"" .. output_deps .. "/lib/\"",
        "{COPY} \"%{cfg.targetdir}/nvrhi.pdb\" \"" .. output_deps .. "/lib/\"",
    }

     filter "configurations:Debug"
         runtime "Debug"
         symbols "on"
         targetname "nvrhi"

     filter "configurations:Release"
         runtime "Release"
         optimize "on"
         targetname "nvrhi"

    filter "platforms:Win64"
        defines
        {
            "NOMINMAX",
            "VK_USE_PLATFORM_WIN32_KHR",
        }
        