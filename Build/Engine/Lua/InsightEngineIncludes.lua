-- simple modules

-- table for our functions
local InsightEngineIncludes = { }

-- Shorthand form is less typing and doesn't use a local variable
function InsightEngineIncludes.AddIncludes(includeMap, prefix)
    includeMap["InsightCore"] = prefix .. "Engine/Core/inc"
    includeMap["InsightMaths"] = prefix .. "Engine/Maths/inc"
    includeMap["InsightPhysics"] = prefix .. "Engine/Physics/inc"
    includeMap["InsightGraphics"] = prefix .. "Engine/Graphics/inc"
    includeMap["InsightInput"] = prefix .. "Engine/Input/inc"
    includeMap["InsightRuntime"] = prefix .. "Engine/Runtime/inc"
    includeMap["InsightEditor"] = prefix .. "Engine/Editor/inc"
end

return InsightEngineIncludes