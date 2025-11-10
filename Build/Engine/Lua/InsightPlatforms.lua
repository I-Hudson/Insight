-- table for our functions
local CommonConfig = dofile("../../../Engine/lua/CommonConfig.lua")
local InsightPlatforms = { }

function InsightPlatforms.SetWindowsPlatformToolset()
    filter { "platforms:Win64 or platforms:UWP" }
        local msvcInstalls = io.open("vsDevCmdVersions.txt", "r")
        for line in msvcInstalls:lines() do
            local splitString = {}
            for i in string.gmatch(line, '([^,]+)') do
                splitString[#splitString + 1] = i
            end 

            if CommonConfig.PathExists(splitString[1]) then
                local msvcToolset = splitString[3]
                toolset(splitString[3])       
                print("Settings MSVC ToolSet to '" .. splitString[3] .. "'")
                break
            end
        end

        --toolset("clang")
end

function InsightPlatforms.All()
    filter { "platforms:Win64 or platforms:UWP" }
        InsightPlatforms.SetWindowsPlatformToolset()

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