-- table for our functions
local InsightPlatforms = { }

function InsightPlatforms.All()
    filter { "platforms:Win64 or platforms:UWP" }
        toolset("msc")
        --toolset("clang")
        defines
        {
            "IS_PLATFORM_WINDOWS",
            "IS_PLATFORM_WIN32",
            "IS_MATHS_DIRECTX_MATHS",
            --"IS_MATHS_CONSTRUCTOR_GLM",
            --"IS_MATHS_GLM",
            "IS_DX12_ENABLED",
            "IS_CPP_WINRT",

            "USE_PIX",
            "NOMINMAX",

            "SPLASH_PLATFORM_WINDOWS",
            
            "VK_USE_PLATFORM_WIN32_KHR",
        }

        if VULKAN_SDK == nil then
        else
            defines
            {
                "IS_VULKAN_ENABLED",
            }
        end

        buildoptions
        {
            "/utf-8"
        }

        links
        {
            "WinPixEventRuntime",
            "cppwinrt_fast_forwarder",
            "Comctl32",
        }
        
    filter { "platforms:Win64", "configurations:Debug or configurations:Testing" }
        links
        {
            "spdlogd",
        }
        ignoredefaultlibraries
        {
            "libcmt",
            "msvcrt",
            "libcmtd",
        }
    filter { "platforms:Win64", "configurations:Release" }
        links
        {
            "spdlog",
        }
        ignoredefaultlibraries
        {
            "libcmt",
            "libcmtd",
            "msvcrtd",
        }

    filter "system:Unix"
    	system "linux"
    	toolset("clang")
        defines
        {
            "IS_PLATFORM_LINUX",
            "IS_VULKAN_ENABLED",
        }
end

return InsightPlatforms