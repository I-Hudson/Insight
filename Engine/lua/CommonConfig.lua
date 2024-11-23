local CommonConfig = { }

local getOsName = require "get_os_name"

function CommonConfig.PostBuildCopyLibraryToOutput()
    filter { "kind:SharedLib or SharedLib" }
    postbuildcommands
    {
        "{COPY} \"%{cfg.targetdir}/%{prj.name}" .. output_project_subfix .. ".dll\" \"%{wks.location}deps/".. outputdir..  "/dll/\"\n",
        "{COPY} \"%{cfg.targetdir}/%{prj.name}" .. output_project_subfix .. ".lib\" \"%{wks.location}deps/".. outputdir..  "/lib/\"\n",
        "{COPY} \"%{cfg.targetdir}/%{prj.name}" .. output_project_subfix .. ".pdb\" \"%{wks.location}deps/".. outputdir..  "/pdb/\"\n",
        "{COPY} \"%{cfg.targetdir}/%{prj.name}" .. output_project_subfix .. ".dll\" \"%{wks.location}bin/".. outputdir..  "/" .. output_executable .. "/\"\n",
    }
filter { "kind:StaticLib or StaticLib" }
    postbuildcommands
    {
        "{COPY} \"%{cfg.targetdir}/%{prj.name}" .. output_project_subfix .. ".lib\" \"%{wks.location}deps/".. outputdir..  "/lib/\"\n",
        "{COPY} \"%{cfg.targetdir}/%{prj.name}" .. output_project_subfix .. ".pdb\" \"%{wks.location}deps/".. outputdir..  "/pdb/\"\n",
    }
end

function CommonConfig.FilterConfigurations()
    filter "configurations:Debug or configurations:Testing"
        defines { "DEBUG" }  
        symbols "On" 
        libdirs
        {
            "%{wks.location}/deps/lib/debug",
        }

    filter "configurations:Release"
        defines { "NDEBUG" }    
        optimize "On" 
        libdirs
        {
            "%{wks.location}/deps/lib/release",
        }
end

-- sBatchFile = .bat for windows, .sh for x
function CommonConfig.OpenCMD(arguments)
    local b = package.cpath:match("%p["..package.config:sub(1,1).."]?%p(%a+)")
    print("Running CMD with Arguments '" .. arguments .. "'")
    local osName, osArch = getOsName.get_os_name()
    print(osName)
    print(osName == "Windows")
    if osName == "Windows" then 
        -- windows
        print("Execute CMD")
        os.execute("\"" .. arguments .. "\"")
    elseif osName == "Linux" then
        -- macos
        os.execute("chmod +x \""..arguments .. "\"")
        os.execute("open -a Terminal.app \""..arguments .. "\"")
    elseif osName == "Mac" then
        -- Linux
        os.execute("chmod +x \"" .. arguments .. "\"")
        os.execute("xterm -hold -e \"" .. arguments .."\" &")
    end 
end 

function CommonConfig.GitClone(branchTag, url, repoDestination)
    local gitCommand = "git clone"
    if (branchTag ~= nil and branchTag ~= "") then
        gitCommand = gitCommand .. " --branch " .. branchTag   
    end
    gitCommand = gitCommand .. " " .. url
    if repoDestination ~= nil and repoDestination ~= "" then
        gitCommand = gitCommand .. " " .. repoDestination
    end
    CommonConfig.OpenCMD(gitCommand)
end

return CommonConfig