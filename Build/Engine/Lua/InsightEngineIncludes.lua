-- simple modules

-- table for our functions
local InsightEngineIncludes = { }

-- Shorthand form is less typing and doesn't use a local variable
function InsightEngineIncludes.AddIncludes(includeMap, prefix)
    includeMap["InsightCore"] = prefix .. "Engine/Core/inc"
    includeMap["InsightMaths"] = prefix .. "Engine/Maths/inc"
    includeMap["InsightInput"] = prefix .. "Engine/Input/inc"
    includeMap["InsightPhysics"] = prefix .. "Engine/Physics/inc"
    includeMap["InsightGraphics"] = prefix .. "Engine/Graphics/inc"
    includeMap["InsightRuntime"] = prefix .. "Engine/Runtime/inc"
    includeMap["InsightEditor"] = prefix .. "Engine/Editor/inc"
end

function InsightEngineIncludes.AddToolsIncludes(includeMap, prefix)
    includeMap["InsightToolAssetPacker"] = prefix .. "Engine/Core/inc"
    includeMap["InsightToolShaderCompiler"] = prefix .. "Engine/Tools/ShaderCompiler/inc"
end

return InsightEngineIncludes