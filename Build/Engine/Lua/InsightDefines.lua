-- table for our functions
local InsightDefines = { }

function InsightDefines.All()
    defines
    {
        "_CRT_SECURE_NO_WARNINGS",
        "_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS",
        "_SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING",
        "_SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS",

        "GLM_FORCE_SWIZZLE",
        "GLM_FORCE_LEFT_HANDED",
        "GLM_FORCE_DEPTH_ZERO_TO_ONE",

        "REFLECT_TYPE_INFO_ENABLED",
        "REFLECT_DLL_IMPORT",

        "IS_PLATFORM_X64",
        "IS_MEMORY_TRACKING",
        "IS_ENGINE",
        "RENDER_GRAPH_ENABLED",
        "TOBJECTPTR_REF_COUNTING",

        "VERTEX_NORMAL_PACKED",
        "VERTEX_COLOUR_PACKED",
        --"VERTEX_UV_PACKED",
        "VERTEX_BONE_ID_PACKED",
        --"VERTEX_BONE_WEIGHT_PACKED",
        --"VERTEX_SPLIT_STREAMS",

        --"RENDERGRAPH_V2_ENABLED",
        --"IS_RESOURCE_HANDLES_ENABLED",

        "SPDLOG_COMPILED_LIB",
        --"SPDLOG_SHARED_LIB",
    }
end

return InsightDefines