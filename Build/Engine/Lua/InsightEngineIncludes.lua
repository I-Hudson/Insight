-- simple modules

-- table for our functions
local InsightEngineIncludes = { }

-- Shorthand form is less typing and doesn't use a local variable
function InsightEngineIncludes.AddIncludes(includeMap)
    includeMap["InsightCore"] = "%{wks.location}Engine/Core/inc"
    includeMap["InsightMaths"] = "%{wks.location}Engine/Maths/inc"
    includeMap["InsightGraphics"] = "%{wks.location}Engine/Graphics/inc"
    includeMap["InsightInput"] = "%{wks.location}Engine/Input/inc"
    includeMap["InsightRuntime"] = "%{wks.location}Engine/Runtime/inc"
    includeMap["InsightEditor"] = "%{wks.location}Engine/Editor/inc"
end

return InsightEngineIncludes