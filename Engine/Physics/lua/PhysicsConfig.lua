local CommonConfig = require "../../lua/CommonConfig"

local PhysicsConfig = { }

local JoltPhysics=1
local PhysXPhysics=2
local EnabledPhysicsEngine=JoltPhysics

-- Check in Insight/Engine/Physics/Physics.lua for the correct int to physics engine. 
function PhysicsConfig.DefinesSharedLib()
    defines
    {
        "IS_EXPORT_PHYSICS_DLL",
    }

    if EnabledPhysicsEngine == JoltPhysics then
        defines
        {
            "IS_PHYSICS_JOLT",
            "JPH_DEBUG_RENDERER",
            "JPH_FLOATING_POINT_EXCEPTIONS_ENABLED",
            "JPH_PROFILE_ENABLED",
            "JPH_OBJECT_STREAM",
        }
    end
    if EnabledPhysicsEngine == PhysXPhysics then
        defines
        {
            "IS_PHYSICS_PHYSX",
        }
    end
end

function PhysicsConfig.DefinesStaticLib()
    defines
    {
    }

    if EnabledPhysicsEngine == JoltPhysics then
        defines
        {
            "IS_PHYSICS_JOLT",
            "JPH_DEBUG_RENDERER",
            "JPH_FLOATING_POINT_EXCEPTIONS_ENABLED",
            "JPH_PROFILE_ENABLED",
            "JPH_OBJECT_STREAM",
        }
    elseif EnabledPhysicsEngine == PhysXPhysics then
        defines
        {
            "IS_PHYSICS_PHYSX",
        }
    end
end

function PhysicsConfig.IncludeDirs()
    includedirs
    {
        "inc",
        "%{IncludeDirs.InsightCore}",
        "%{IncludeDirs.InsightMaths}",
        
        "%{IncludeDirs.spdlog}",
        "%{IncludeDirs.imgui}",
        "%{IncludeDirs.reflect}",
        "%{IncludeDirs.JoltPhysics}",
        "%{IncludeDirs.Physx}",
    }
end

function PhysicsConfig.LibraryDirs()
    libdirs
    {
        "%{wks.location}/deps/lib",
    }
end

function PhysicsConfig.LibraryLinks()
    links
    {
        "Insight_Core" .. output_project_subfix .. ".lib",
        "Insight_Maths" .. output_project_subfix .. ".lib",
        "imgui.lib",
    }

    if EnabledPhysicsEngine == JoltPhysics then
        links
        {
            "Jolt.lib",
        }
    elseif EnabledPhysicsEngine == PhysXPhysics then
        links
        {
            "PhysXCommon_64.lib",
            "PhysX_64.lib",
            "PhysXFoundation_64.lib",
        }
    end
end

function PhysicsConfig.PreBuildCommands(OutputDir)
    filter "configurations:Debug or configurations:Testing"
        if EnabledPhysicsEngine == JoltPhysics then
            prebuildcommands
            {
                "{COPY} \"%{wks.location}vendor/JoltPhysics/Build/VS2022_CL/Debug/Jolt.lib\" \"%{wks.location}deps/".. OutputDir..  "/lib/\"\n",
            }
        elseif EnabledPhysicsEngine == PhysXPhysics then
            prebuildcommands
            {
                "{COPY} \"%{wks.location}vendor/PhysX/physx/bin/win.x86_64.vc143.md/checked/PhysXCommon_64.lib\" \"%{wks.location}deps/".. OutputDir..  "/lib/\"\n",
                "{COPY} \"%{wks.location}vendor/PhysX/physx/bin/win.x86_64.vc143.md/checked/PhysX_64.lib\" \"%{wks.location}deps/".. OutputDir..  "/lib/\"\n",
                "{COPY} \"%{wks.location}vendor/PhysX/physx/bin/win.x86_64.vc143.md/checked/PhysXFoundation_64.lib\" \"%{wks.location}deps/".. OutputDir..  "/lib/\"\n",

                "{COPY} \"%{wks.location}vendor/PhysX/physx/bin/win.x86_64.vc143.md/checked/PhysXCommon_64.dll\" \"%{wks.location}deps/".. OutputDir..  "/dll/\"\n",
                "{COPY} \"%{wks.location}vendor/PhysX/physx/bin/win.x86_64.vc143.md/checked/PhysX_64.dll\" \"%{wks.location}deps/".. OutputDir..  "/dll/\"\n",
                "{COPY} \"%{wks.location}vendor/PhysX/physx/bin/win.x86_64.vc143.md/checked/PhysXFoundation_64.dll\" \"%{wks.location}deps/".. OutputDir..  "/dll/\"\n",
            }
        end

    filter "configurations:Release"
        if EnabledPhysicsEngine == JoltPhysics then
            prebuildcommands
            {
                "{COPY} \"%{wks.location}vendor/JoltPhysics/Build/VS2022_CL/Release/Jolt.lib\" \"%{wks.location}deps/".. OutputDir..  "/lib/\"\n",
            }
        end

        if EnabledPhysicsEngine == PhysXPhysics then
            prebuildcommands
            {
                "{COPY} \"%{wks.location}vendor/PhysX/physx/bin/win.x86_64.vc143.md/release/PhysXCommon_64.lib\" \"%{wks.location}deps/".. OutputDir..  "/lib/\"\n",
                "{COPY} \"%{wks.location}vendor/PhysX/physx/bin/win.x86_64.vc143.md/release/PhysX_64.lib\" \"%{wks.location}deps/".. OutputDir..  "/lib/\"\n",
                "{COPY} \"%{wks.location}vendor/PhysX/physx/bin/win.x86_64.vc143.md/release/PhysXFoundation_64.lib\" \"%{wks.location}deps/".. OutputDir..  "/lib/\"\n",

                "{COPY} \"%{wks.location}vendor/PhysX/physx/bin/win.x86_64.vc143.md/release/PhysXCommon_64.dll\" \"%{wks.location}deps/".. OutputDir..  "/dll/\"\n",
                "{COPY} \"%{wks.location}vendor/PhysX/physx/bin/win.x86_64.vc143.md/release/PhysX_64.dll\" \"%{wks.location}deps/".. OutputDir..  "/dll/\"\n",
                "{COPY} \"%{wks.location}vendor/PhysX/physx/bin/win.x86_64.vc143.md/release/PhysXFoundation_64.dll\" \"%{wks.location}deps/".. OutputDir..  "/dll/\"\n",
            }
        end
end


return PhysicsConfig