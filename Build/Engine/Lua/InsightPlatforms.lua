-- table for our functions
local InsightPlatforms = { }

function InsightPlatforms.All()
    filter { "platforms:Win64" or "platforms:UWP" }
        toolset("msc-v143")
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

        links
        {
            "WinPixEventRuntime.lib",
            "cppwinrt_fast_forwarder.lib",
            "Comctl32.lib",
            "spdlog.lib",
        }

    filter { "platforms:Win64", "configurations:Debug" or "configurations:Testing" }
        ignoredefaultlibraries
        {
            "libcmt.lib",
            "msvcrt.lib",
            "libcmtd.lib",
        }
    filter { "platforms:Win64", "configurations:Release" }
        ignoredefaultlibraries
        {
            "libcmt.lib",
            "libcmtd.lib",
            "msvcrtd.lib",
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