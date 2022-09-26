project "assimp"  
    kind "StaticLib"   
    language "C++"
    cppdialect "C++17"
    configurations { "Debug", "Release" }
    
    targetdir (outputdir_target .. "/%{prj.name}")
    objdir (outputdir_obj.. "/%{prj.name}")
    debugdir (outputdir_debug .. "/%{prj.name}")

    folderDirAssimp = "../vendor/assimp/"
	location "%{folderDirAssimp}"
    
    defines 
    {
        "RAPIDJSON_HAS_STDSTRING",
        "OPENDDL_STATIC_LIBARY",

        "ASSIMP_BUILD_NO_OWN_ZLIB",
  
        "ASSIMP_BUILD_NO_X_IMPORTER",
        --"ASSIMP_BUILD_NO_3DS_IMPORTER",
        "ASSIMP_BUILD_NO_MD3_IMPORTER",
        "ASSIMP_BUILD_NO_MDL_IMPORTER",
        "ASSIMP_BUILD_NO_MD2_IMPORTER",
        -- "ASSIMP_BUILD_NO_PLY_IMPORTER",
        "ASSIMP_BUILD_NO_ASE_IMPORTER",
        -- "ASSIMP_BUILD_NO_OBJ_IMPORTER",
        "ASSIMP_BUILD_NO_AMF_IMPORTER",
        "ASSIMP_BUILD_NO_HMP_IMPORTER",
        "ASSIMP_BUILD_NO_SMD_IMPORTER",
        "ASSIMP_BUILD_NO_MDC_IMPORTER",
        "ASSIMP_BUILD_NO_MD5_IMPORTER",
        "ASSIMP_BUILD_NO_STL_IMPORTER",
        "ASSIMP_BUILD_NO_LWO_IMPORTER",
        "ASSIMP_BUILD_NO_DXF_IMPORTER",
        "ASSIMP_BUILD_NO_NFF_IMPORTER",
        "ASSIMP_BUILD_NO_RAW_IMPORTER",
        "ASSIMP_BUILD_NO_OFF_IMPORTER",
        "ASSIMP_BUILD_NO_AC_IMPORTER",
        "ASSIMP_BUILD_NO_BVH_IMPORTER",
        "ASSIMP_BUILD_NO_IRRMESH_IMPORTER",
        "ASSIMP_BUILD_NO_IRR_IMPORTER",
        "ASSIMP_BUILD_NO_Q3D_IMPORTER",
        "ASSIMP_BUILD_NO_B3D_IMPORTER",
        -- "ASSIMP_BUILD_NO_COLLADA_IMPORTER",
        "ASSIMP_BUILD_NO_TERRAGEN_IMPORTER",
        "ASSIMP_BUILD_NO_CSM_IMPORTER",
        "ASSIMP_BUILD_NO_3D_IMPORTER",
        "ASSIMP_BUILD_NO_LWS_IMPORTER",
        "ASSIMP_BUILD_NO_OGRE_IMPORTER",
        "ASSIMP_BUILD_NO_OPENGEX_IMPORTER",
        "ASSIMP_BUILD_NO_MS3D_IMPORTER",
        "ASSIMP_BUILD_NO_COB_IMPORTER",
        "ASSIMP_BUILD_NO_BLEND_IMPORTER",
        "ASSIMP_BUILD_NO_Q3BSP_IMPORTER",
        "ASSIMP_BUILD_NO_NDO_IMPORTER",
        "ASSIMP_BUILD_NO_IFC_IMPORTER",
        "ASSIMP_BUILD_NO_XGL_IMPORTER",
        --"ASSIMP_BUILD_NO_FBX_IMPORTER",
        "ASSIMP_BUILD_NO_ASSBIN_IMPORTER",
        -- "ASSIMP_BUILD_NO_GLTF_IMPORTER",
        "ASSIMP_BUILD_NO_C4D_IMPORTER",
        "ASSIMP_BUILD_NO_3MF_IMPORTER",
        "ASSIMP_BUILD_NO_X3D_IMPORTER",
        "ASSIMP_BUILD_NO_MMD_IMPORTER",
        
        "ASSIMP_BUILD_NO_STEP_EXPORTER",
        "ASSIMP_BUILD_NO_SIB_IMPORTER",
  
        -- "ASSIMP_BUILD_NO_MAKELEFTHANDED_PROCESS",
        -- "ASSIMP_BUILD_NO_FLIPUVS_PROCESS",
        -- "ASSIMP_BUILD_NO_FLIPWINDINGORDER_PROCESS",
        -- "ASSIMP_BUILD_NO_CALCTANGENTS_PROCESS",
        "ASSIMP_BUILD_NO_JOINVERTICES_PROCESS",
        -- "ASSIMP_BUILD_NO_TRIANGULATE_PROCESS",
        "ASSIMP_BUILD_NO_GENFACENORMALS_PROCESS",
        -- "ASSIMP_BUILD_NO_GENVERTEXNORMALS_PROCESS",
        "ASSIMP_BUILD_NO_REMOVEVC_PROCESS",
        "ASSIMP_BUILD_NO_SPLITLARGEMESHES_PROCESS",
        "ASSIMP_BUILD_NO_PRETRANSFORMVERTICES_PROCESS",
        "ASSIMP_BUILD_NO_LIMITBONEWEIGHTS_PROCESS",
        -- "ASSIMP_BUILD_NO_VALIDATEDS_PROCESS",
        --"ASSIMP_BUILD_NO_IMPROVECACHELOCALITY_PROCESS",
        --"ASSIMP_BUILD_NO_FIXINFACINGNORMALS_PROCESS",
        --"ASSIMP_BUILD_NO_REMOVE_REDUNDANTMATERIALS_PROCESS",
        "ASSIMP_BUILD_NO_FINDINVALIDDATA_PROCESS",
        "ASSIMP_BUILD_NO_FINDDEGENERATES_PROCESS",
        "ASSIMP_BUILD_NO_SORTBYPTYPE_PROCESS",
        --"ASSIMP_BUILD_NO_GENUVCOORDS_PROCESS",
        "ASSIMP_BUILD_NO_TRANSFORMTEXCOORDS_PROCESS",
        "ASSIMP_BUILD_NO_FINDINSTANCES_PROCESS",
        --"ASSIMP_BUILD_NO_OPTIMIZEMESHES_PROCESS",
        --"ASSIMP_BUILD_NO_OPTIMIZEGRAPH_PROCESS",
        "ASSIMP_BUILD_NO_SPLITBYBONECOUNT_PROCESS",
        "ASSIMP_BUILD_NO_DEBONE_PROCESS",
        "ASSIMP_BUILD_NO_EMBEDTEXTURES_PROCESS",
        "ASSIMP_BUILD_NO_GLOBALSCALE_PROCESS",
    }
    
    files 
    {
        folderDirAssimp .. "include/**",
        folderDirAssimp .. "code/**.cpp",
        folderDirAssimp .. "code/**.h",

        folderDirAssimp .. "contrib/Clipper/**.cpp",
        folderDirAssimp .. "contrib/Clipper/**.hpp",

        folderDirAssimp .. "contrib/opem3dgc/**.cpp",
        folderDirAssimp .. "contrib/opem3dgc/**.h",
        folderDirAssimp .. "contrib/opem3dgc/**.inl",

        folderDirAssimp .. "contrib/openddlparser/**.cpp",
        folderDirAssimp .. "contrib/openddlparser/**.h",

        folderDirAssimp .. "contrib/Poly2Tri/**.cc",
        folderDirAssimp .. "contrib/Poly2Tri/**.h",

        folderDirAssimp .. "contrib/Pugixml/**.cpp",
        folderDirAssimp .. "contrib/Pugixml/**.hpp",

        folderDirAssimp .. "contrib/stb/**.h",

        folderDirAssimp .. "contrib/unzip/**.c",
        folderDirAssimp .. "contrib/unzip/**.h",

        folderDirAssimp .. "contrib/zlib/*.c",
        folderDirAssimp .. "contrib/zlip/*.h",
    }

    includedirs 
    {
        folderDirAssimp .. "include",
        
        folderDirAssimp .. "code",
        folderDirAssimp .. "code/AssetLib",
        folderDirAssimp .. "code/CApi",
        folderDirAssimp .. "code/Common",
        folderDirAssimp .. "code/Material",
        folderDirAssimp .. "code/Pbrt",
        folderDirAssimp .. "code/PostProcessing",
        folderDirAssimp .. "code/res",

        folderDirAssimp, 
        folderDirAssimp .. "contrib",
        folderDirAssimp .. "contrib/openddlparser/include",
        folderDirAssimp .. "contrib/irrXML",
        folderDirAssimp .. "contrib/pugixml/src",
        folderDirAssimp .. "contrib/stb",
        folderDirAssimp .. "contrib/unzip",
        folderDirAssimp .. "contrib/utf8cpp",
        folderDirAssimp .. "contrib/zip",
        folderDirAssimp .. "contrib/zlib",
        folderDirAssimp .. "contrib/rapidjson/include",
    }

     filter  "configurations:Debug"
         runtime "Debug"
         symbols "on"
         targetname "assimpd"
         postbuildcommands
         {
             "{COPY} \"%{cfg.targetdir}/assimpd.lib\" \"" .. output_deps .. "/lib/\"",
         }

     filter  "configurations:Release"
         runtime "Release"
         optimize "on"
         targetname "assimp"
        postbuildcommands
        {
            "{COPY} \"%{cfg.targetdir}/assimp.lib\" \"" .. output_deps .. "/lib/\"",
        }

premake.modules.lua = {}
local m = premake.modules.lua
local p = premake

function renameAndDeleteFile(oldPath, newPath)
    ok, err = os.rename(oldPath, newPath)
    print("File renamed: " .. err)
    ok, err = os.remove("premakeFiles/" .. oldPath)
    print("File deleted: " .. err)
end

function m.createAssimpConfig()
    p.w([[
/*
---------------------------------------------------------------------------
Open Asset Import Library (assimp)
---------------------------------------------------------------------------

Copyright (c) 2006-2020, assimp team


All rights reserved.

Redistribution and use of this software in source and binary forms,
with or without modification, are permitted provided that the following
conditions are met:

* Redistributions of source code must retain the above
  copyright notice, this list of conditions and the
  following disclaimer.

* Redistributions in binary form must reproduce the above
  copyright notice, this list of conditions and the
  following disclaimer in the documentation and/or other
  materials provided with the distribution.

* Neither the name of the assimp team, nor the names of its
  contributors may be used to endorse or promote products
  derived from this software without specific prior
  written permission of the assimp team.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
---------------------------------------------------------------------------
*/

/** @file config.h
 *  @brief Defines constants for configurable properties for the library
 *
 *  Typically these properties are set via
 *  #Assimp::Importer::SetPropertyFloat,
 *  #Assimp::Importer::SetPropertyInteger or
 *  #Assimp::Importer::SetPropertyString,
 *  depending on the data type of a property. All properties have a
 *  default value. See the doc for the mentioned methods for more details.
 *
 *  <br><br>
 *  The corresponding functions for use with the plain-c API are:
 *  #aiSetImportPropertyInteger,
 *  #aiSetImportPropertyFloat,
 *  #aiSetImportPropertyString
 */
#pragma once
#ifndef AI_CONFIG_H_INC
#define AI_CONFIG_H_INC


// ###########################################################################
// LIBRARY SETTINGS
// General, global settings
// ###########################################################################

// ---------------------------------------------------------------------------
/** @brief Enables time measurements.
 *
 *  If enabled, measures the time needed for each part of the loading
 *  process (i.e. IO time, importing, postprocessing, ..) and dumps
 *  these timings to the DefaultLogger. See the @link perf Performance
 *  Page@endlink for more information on this topic.
 *
 * Property type: bool. Default value: false.
 */
#define AI_CONFIG_GLOB_MEASURE_TIME  \
    "GLOB_MEASURE_TIME"


// ---------------------------------------------------------------------------
/** @brief Global setting to disable generation of skeleton dummy meshes
 *
 * Skeleton dummy meshes are generated as a visualization aid in cases which
 * the input data contains no geometry, but only animation data.
 * Property data type: bool. Default value: false
 */
// ---------------------------------------------------------------------------
#define AI_CONFIG_IMPORT_NO_SKELETON_MESHES \
    "IMPORT_NO_SKELETON_MESHES"



# if 0 // not implemented yet
// ---------------------------------------------------------------------------
/** @brief Set Assimp's multithreading policy.
 *
 * This setting is ignored if Assimp was built without boost.thread
 * support (ASSIMP_BUILD_NO_THREADING, which is implied by ASSIMP_BUILD_BOOST_WORKAROUND).
 * Possible values are: -1 to let Assimp decide what to do, 0 to disable
 * multithreading entirely and any number larger than 0 to force a specific
 * number of threads. Assimp is always free to ignore this settings, which is
 * merely a hint. Usually, the default value (-1) will be fine. However, if
 * Assimp is used concurrently from multiple user threads, it might be useful
 * to limit each Importer instance to a specific number of cores.
 *
 * For more information, see the @link threading Threading page@endlink.
 * Property type: int, default value: -1.
 */
#define AI_CONFIG_GLOB_MULTITHREADING  \
    "GLOB_MULTITHREADING"
#endif

// ###########################################################################
// POST PROCESSING SETTINGS
// Various stuff to fine-tune the behavior of a specific post processing step.
// ###########################################################################


// ---------------------------------------------------------------------------
/** @brief Maximum bone count per mesh for the SplitbyBoneCount step.
 *
 * Meshes are split until the maximum number of bones is reached. The default
 * value is AI_SBBC_DEFAULT_MAX_BONES, which may be altered at
 * compile-time.
 * Property data type: integer.
 */
// ---------------------------------------------------------------------------
#define AI_CONFIG_PP_SBBC_MAX_BONES \
    "PP_SBBC_MAX_BONES"


// default limit for bone count
#if (!defined AI_SBBC_DEFAULT_MAX_BONES)
#   define AI_SBBC_DEFAULT_MAX_BONES        60
#endif


// ---------------------------------------------------------------------------
/** @brief  Specifies the maximum angle that may be between two vertex tangents
 *         that their tangents and bi-tangents are smoothed.
 *
 * This applies to the CalcTangentSpace-Step. The angle is specified
 * in degrees. The maximum value is 175.
 * Property type: float. Default value: 45 degrees
 */
#define AI_CONFIG_PP_CT_MAX_SMOOTHING_ANGLE \
    "PP_CT_MAX_SMOOTHING_ANGLE"

// ---------------------------------------------------------------------------
/** @brief Source UV channel for tangent space computation.
 *
 * The specified channel must exist or an error will be raised.
 * Property type: integer. Default value: 0
 */
// ---------------------------------------------------------------------------
#define AI_CONFIG_PP_CT_TEXTURE_CHANNEL_INDEX \
    "PP_CT_TEXTURE_CHANNEL_INDEX"

// ---------------------------------------------------------------------------
/** @brief  Specifies the maximum angle that may be between two face normals
 *          at the same vertex position that their are smoothed together.
 *
 * Sometimes referred to as 'crease angle'.
 * This applies to the GenSmoothNormals-Step. The angle is specified
 * in degrees, so 180 is PI. The default value is 175 degrees (all vertex
 * normals are smoothed). The maximum value is 175, too. Property type: float.
 * Warning: setting this option may cause a severe loss of performance. The
 * performance is unaffected if the #AI_CONFIG_FAVOUR_SPEED flag is set but
 * the output quality may be reduced.
 */
#define AI_CONFIG_PP_GSN_MAX_SMOOTHING_ANGLE \
    "PP_GSN_MAX_SMOOTHING_ANGLE"


// ---------------------------------------------------------------------------
/** @brief Sets the colormap (= palette) to be used to decode embedded
 *         textures in MDL (Quake or 3DGS) files.
 *
 * This must be a valid path to a file. The file is 768 (256*3) bytes
 * large and contains RGB triplets for each of the 256 palette entries.
 * The default value is colormap.lmp. If the file is not found,
 * a default palette (from Quake 1) is used.
 * Property type: string.
 */
#define AI_CONFIG_IMPORT_MDL_COLORMAP       \
    "IMPORT_MDL_COLORMAP"

// ---------------------------------------------------------------------------
/** @brief Configures the #aiProcess_RemoveRedundantMaterials step to
 *  keep materials matching a name in a given list.
 *
 * This is a list of 1 to n strings, ' ' serves as delimiter character.
 * Identifiers containing whitespaces must be enclosed in *single*
 * quotation marks. For example:<tt>
 * "keep-me and_me_to anotherMaterialToBeKept \'name with whitespace\'"</tt>.
 * If a material matches on of these names, it will not be modified or
 * removed by the postprocessing step nor will other materials be replaced
 * by a reference to it. <br>
 * This option might be useful if you are using some magic material names
 * to pass additional semantics through the content pipeline. This ensures
 * they won't be optimized away, but a general optimization is still
 * performed for materials not contained in the list.
 * Property type: String. Default value: n/a
 * @note Linefeeds, tabs or carriage returns are treated as whitespace.
 *   Material names are case sensitive.
 */
#define AI_CONFIG_PP_RRM_EXCLUDE_LIST   \
    "PP_RRM_EXCLUDE_LIST"

// ---------------------------------------------------------------------------
/** @brief Configures the #aiProcess_PreTransformVertices step to
 *  keep the scene hierarchy. Meshes are moved to worldspace, but
 *  no optimization is performed (read: meshes with equal materials are not
 *  joined. The total number of meshes won't change).
 *
 * This option could be of use for you if the scene hierarchy contains
 * important additional information which you intend to parse.
 * For rendering, you can still render all meshes in the scene without
 * any transformations.
 * Property type: bool. Default value: false.
 */
#define AI_CONFIG_PP_PTV_KEEP_HIERARCHY     \
    "PP_PTV_KEEP_HIERARCHY"

// ---------------------------------------------------------------------------
/** @brief Configures the #aiProcess_PreTransformVertices step to normalize
 *  all vertex components into the [-1,1] range. That is, a bounding box
 *  for the whole scene is computed, the maximum component is taken and all
 *  meshes are scaled appropriately (uniformly of course!).
 *  This might be useful if you don't know the spatial dimension of the input
 *  data*/
#define AI_CONFIG_PP_PTV_NORMALIZE  \
    "PP_PTV_NORMALIZE"

// ---------------------------------------------------------------------------
/** @brief Configures the #aiProcess_PreTransformVertices step to use
 *  a users defined matrix as the scene root node transformation before
 *  transforming vertices.
 *  Property type: bool. Default value: false.
 */
#define AI_CONFIG_PP_PTV_ADD_ROOT_TRANSFORMATION    \
    "PP_PTV_ADD_ROOT_TRANSFORMATION"

// ---------------------------------------------------------------------------
/** @brief Configures the #aiProcess_PreTransformVertices step to use
 *  a users defined matrix as the scene root node transformation before
 *  transforming vertices. This property correspond to the 'a1' component
 *  of the transformation matrix.
 *  Property type: aiMatrix4x4.
 */
#define AI_CONFIG_PP_PTV_ROOT_TRANSFORMATION    \
    "PP_PTV_ROOT_TRANSFORMATION"

// ---------------------------------------------------------------------------
/** @brief Configures the #aiProcess_FindDegenerates step to
 *  remove degenerated primitives from the import - immediately.
 *
 * The default behaviour converts degenerated triangles to lines and
 * degenerated lines to points. See the documentation to the
 * #aiProcess_FindDegenerates step for a detailed example of the various ways
 * to get rid of these lines and points if you don't want them.
 * Property type: bool. Default value: false.
 */
#define AI_CONFIG_PP_FD_REMOVE \
    "PP_FD_REMOVE"

// ---------------------------------------------------------------------------
/**
 *  @brief  Configures the #aiProcess_FindDegenerates to check the area of a
 *  trinagle to be greates than e-6. If this is not the case the triangle will
 *  be removed if #AI_CONFIG_PP_FD_REMOVE is set to true.
 */
#define AI_CONFIG_PP_FD_CHECKAREA \
    "PP_FD_CHECKAREA"

// ---------------------------------------------------------------------------
/** @brief Configures the #aiProcess_OptimizeGraph step to preserve nodes
 * matching a name in a given list.
 *
 * This is a list of 1 to n strings, ' ' serves as delimiter character.
 * Identifiers containing whitespaces must be enclosed in *single*
 * quotation marks. For example:<tt>
 * "keep-me and_me_to anotherNodeToBeKept \'name with whitespace\'"</tt>.
 * If a node matches on of these names, it will not be modified or
 * removed by the postprocessing step.<br>
 * This option might be useful if you are using some magic node names
 * to pass additional semantics through the content pipeline. This ensures
 * they won't be optimized away, but a general optimization is still
 * performed for nodes not contained in the list.
 * Property type: String. Default value: n/a
 * @note Linefeeds, tabs or carriage returns are treated as whitespace.
 *   Node names are case sensitive.
 */
#define AI_CONFIG_PP_OG_EXCLUDE_LIST    \
    "PP_OG_EXCLUDE_LIST"

// ---------------------------------------------------------------------------
/** @brief  Set the maximum number of triangles in a mesh.
 *
 * This is used by the "SplitLargeMeshes" PostProcess-Step to determine
 * whether a mesh must be split or not.
 * @note The default value is AI_SLM_DEFAULT_MAX_TRIANGLES
 * Property type: integer.
 */
#define AI_CONFIG_PP_SLM_TRIANGLE_LIMIT \
    "PP_SLM_TRIANGLE_LIMIT"

// default value for AI_CONFIG_PP_SLM_TRIANGLE_LIMIT
#if (!defined AI_SLM_DEFAULT_MAX_TRIANGLES)
#   define AI_SLM_DEFAULT_MAX_TRIANGLES     1000000
#endif

// ---------------------------------------------------------------------------
/** @brief  Set the maximum number of vertices in a mesh.
 *
 * This is used by the "SplitLargeMeshes" PostProcess-Step to determine
 * whether a mesh must be split or not.
 * @note The default value is AI_SLM_DEFAULT_MAX_VERTICES
 * Property type: integer.
 */
#define AI_CONFIG_PP_SLM_VERTEX_LIMIT \
    "PP_SLM_VERTEX_LIMIT"

// default value for AI_CONFIG_PP_SLM_VERTEX_LIMIT
#if (!defined AI_SLM_DEFAULT_MAX_VERTICES)
#   define AI_SLM_DEFAULT_MAX_VERTICES      1000000
#endif

// ---------------------------------------------------------------------------
/** @brief Set the maximum number of bones affecting a single vertex
 *
 * This is used by the #aiProcess_LimitBoneWeights PostProcess-Step.
 * @note The default value is AI_LMW_MAX_WEIGHTS
 * Property type: integer.*/
#define AI_CONFIG_PP_LBW_MAX_WEIGHTS    \
    "PP_LBW_MAX_WEIGHTS"

// default value for AI_CONFIG_PP_LBW_MAX_WEIGHTS
#if (!defined AI_LMW_MAX_WEIGHTS)
#   define AI_LMW_MAX_WEIGHTS   0x4
#endif // !! AI_LMW_MAX_WEIGHTS

// ---------------------------------------------------------------------------
/** @brief Lower the deboning threshold in order to remove more bones.
 *
 * This is used by the #aiProcess_Debone PostProcess-Step.
 * @note The default value is AI_DEBONE_THRESHOLD
 * Property type: float.*/
#define AI_CONFIG_PP_DB_THRESHOLD \
    "PP_DB_THRESHOLD"

// default value for AI_CONFIG_PP_LBW_MAX_WEIGHTS
#if (!defined AI_DEBONE_THRESHOLD)
#   define AI_DEBONE_THRESHOLD  1.0f
#endif // !! AI_DEBONE_THRESHOLD

// ---------------------------------------------------------------------------
/** @brief Require all bones qualify for deboning before removing any
 *
 * This is used by the #aiProcess_Debone PostProcess-Step.
 * @note The default value is 0
 * Property type: bool.*/
#define AI_CONFIG_PP_DB_ALL_OR_NONE \
    "PP_DB_ALL_OR_NONE"

/** @brief Default value for the #AI_CONFIG_PP_ICL_PTCACHE_SIZE property
 */
#ifndef PP_ICL_PTCACHE_SIZE
#   define PP_ICL_PTCACHE_SIZE 12
#endif

// ---------------------------------------------------------------------------
/** @brief Set the size of the post-transform vertex cache to optimize the
 *    vertices for. This configures the #aiProcess_ImproveCacheLocality step.
 *
 * The size is given in vertices. Of course you can't know how the vertex
 * format will exactly look like after the import returns, but you can still
 * guess what your meshes will probably have.
 * @note The default value is #PP_ICL_PTCACHE_SIZE. That results in slight
 * performance improvements for most nVidia/AMD cards since 2002.
 * Property type: integer.
 */
#define AI_CONFIG_PP_ICL_PTCACHE_SIZE   "PP_ICL_PTCACHE_SIZE"

// ---------------------------------------------------------------------------
/** @brief Enumerates components of the aiScene and aiMesh data structures
 *  that can be excluded from the import using the #aiProcess_RemoveComponent step.
 *
 *  See the documentation to #aiProcess_RemoveComponent for more details.
 */
enum aiComponent
{
    /** Normal vectors */
#ifdef SWIG
    aiComponent_NORMALS = 0x2,
#else
    aiComponent_NORMALS = 0x2u,
#endif

    /** Tangents and bitangents go always together ... */
#ifdef SWIG
    aiComponent_TANGENTS_AND_BITANGENTS = 0x4,
#else
    aiComponent_TANGENTS_AND_BITANGENTS = 0x4u,
#endif

    /** ALL color sets
     * Use aiComponent_COLORn(N) to specify the N'th set */
    aiComponent_COLORS = 0x8,

    /** ALL texture UV sets
     * aiComponent_TEXCOORDn(N) to specify the N'th set  */
    aiComponent_TEXCOORDS = 0x10,

    /** Removes all bone weights from all meshes.
     * The scenegraph nodes corresponding to the bones are NOT removed.
     * use the #aiProcess_OptimizeGraph step to do this */
    aiComponent_BONEWEIGHTS = 0x20,

    /** Removes all node animations (aiScene::mAnimations).
     * The corresponding scenegraph nodes are NOT removed.
     * use the #aiProcess_OptimizeGraph step to do this */
    aiComponent_ANIMATIONS = 0x40,

    /** Removes all embedded textures (aiScene::mTextures) */
    aiComponent_TEXTURES = 0x80,

    /** Removes all light sources (aiScene::mLights).
     * The corresponding scenegraph nodes are NOT removed.
     * use the #aiProcess_OptimizeGraph step to do this */
    aiComponent_LIGHTS = 0x100,

    /** Removes all cameras (aiScene::mCameras).
     * The corresponding scenegraph nodes are NOT removed.
     * use the #aiProcess_OptimizeGraph step to do this */
    aiComponent_CAMERAS = 0x200,

    /** Removes all meshes (aiScene::mMeshes). */
    aiComponent_MESHES = 0x400,

    /** Removes all materials. One default material will
     * be generated, so aiScene::mNumMaterials will be 1. */
    aiComponent_MATERIALS = 0x800,


    /** This value is not used. It is just there to force the
     *  compiler to map this enum to a 32 Bit integer. */
#ifndef SWIG
    _aiComponent_Force32Bit = 0x9fffffff
#endif
};

// Remove a specific color channel 'n'
#define aiComponent_COLORSn(n) (1u << (n+20u))

// Remove a specific UV channel 'n'
#define aiComponent_TEXCOORDSn(n) (1u << (n+25u))

// ---------------------------------------------------------------------------
/** @brief Input parameter to the #aiProcess_RemoveComponent step:
 *  Specifies the parts of the data structure to be removed.
 *
 * See the documentation to this step for further details. The property
 * is expected to be an integer, a bitwise combination of the
 * #aiComponent flags defined above in this header. The default
 * value is 0. Important: if no valid mesh is remaining after the
 * step has been executed (e.g you thought it was funny to specify ALL
 * of the flags defined above) the import FAILS. Mainly because there is
 * no data to work on anymore ...
 */
#define AI_CONFIG_PP_RVC_FLAGS              \
    "PP_RVC_FLAGS"

// ---------------------------------------------------------------------------
/** @brief Input parameter to the #aiProcess_SortByPType step:
 *  Specifies which primitive types are removed by the step.
 *
 *  This is a bitwise combination of the aiPrimitiveType flags.
 *  Specifying all of them is illegal, of course. A typical use would
 *  be to exclude all line and point meshes from the import. This
 *  is an integer property, its default value is 0.
 */
#define AI_CONFIG_PP_SBP_REMOVE             \
    "PP_SBP_REMOVE"

// ---------------------------------------------------------------------------
/** @brief Input parameter to the #aiProcess_FindInvalidData step:
 *  Specifies the floating-point accuracy for animation values. The step
 *  checks for animation tracks where all frame values are absolutely equal
 *  and removes them. This tweakable controls the epsilon for floating-point
 *  comparisons - two keys are considered equal if the invariant
 *  abs(n0-n1)>epsilon holds true for all vector respectively quaternion
 *  components. The default value is 0.f - comparisons are exact then.
 */
#define AI_CONFIG_PP_FID_ANIM_ACCURACY              \
    "PP_FID_ANIM_ACCURACY"

// ---------------------------------------------------------------------------
/** @brief Input parameter to the #aiProcess_FindInvalidData step:
 *  Set to true to ignore texture coordinates. This may be useful if you have
 *  to assign different kind of textures like one for the summer or one for the winter.
 */
#define AI_CONFIG_PP_FID_IGNORE_TEXTURECOORDS        \
    "PP_FID_IGNORE_TEXTURECOORDS"

// TransformUVCoords evaluates UV scalings
#define AI_UVTRAFO_SCALING 0x1

// TransformUVCoords evaluates UV rotations
#define AI_UVTRAFO_ROTATION 0x2

// TransformUVCoords evaluates UV translation
#define AI_UVTRAFO_TRANSLATION 0x4

// Everything baked together -> default value
#define AI_UVTRAFO_ALL (AI_UVTRAFO_SCALING | AI_UVTRAFO_ROTATION | AI_UVTRAFO_TRANSLATION)

// ---------------------------------------------------------------------------
/** @brief Input parameter to the #aiProcess_TransformUVCoords step:
 *  Specifies which UV transformations are evaluated.
 *
 *  This is a bitwise combination of the AI_UVTRAFO_XXX flags (integer
 *  property, of course). By default all transformations are enabled
 * (AI_UVTRAFO_ALL).
 */
#define AI_CONFIG_PP_TUV_EVALUATE               \
    "PP_TUV_EVALUATE"

// ---------------------------------------------------------------------------
/** @brief A hint to assimp to favour speed against import quality.
 *
 * Enabling this option may result in faster loading, but it needn't.
 * It represents just a hint to loaders and post-processing steps to use
 * faster code paths, if possible.
 * This property is expected to be an integer, != 0 stands for true.
 * The default value is 0.
 */
#define AI_CONFIG_FAVOUR_SPEED              \
 "FAVOUR_SPEED"


// ###########################################################################
// IMPORTER SETTINGS
// Various stuff to fine-tune the behaviour of specific importer plugins.
// ###########################################################################

// ---------------------------------------------------------------------------
/** @brief Importers which parse JSON may use this to obtain a pointer to a
 * rapidjson::IRemoteSchemaDocumentProvider.
 *
 * The default value is nullptr
 * Property type: void*
 */
#define AI_CONFIG_IMPORT_SCHEMA_DOCUMENT_PROVIDER \
    "IMPORT_SCHEMA_DOCUMENT_PROVIDER"

// ---------------------------------------------------------------------------
/** @brief Set whether the fbx importer will merge all geometry layers present
 *    in the source file or take only the first.
 *
 * The default value is true (1)
 * Property type: bool
 */
#define AI_CONFIG_IMPORT_FBX_READ_ALL_GEOMETRY_LAYERS \
    "IMPORT_FBX_READ_ALL_GEOMETRY_LAYERS"

// ---------------------------------------------------------------------------
/** @brief Set whether the fbx importer will read all materials present in the
 *    source file or take only the referenced materials.
 *
 * This is void unless IMPORT_FBX_READ_MATERIALS=1.
 *
 * The default value is false (0)
 * Property type: bool
 */
#define AI_CONFIG_IMPORT_FBX_READ_ALL_MATERIALS \
    "IMPORT_FBX_READ_ALL_MATERIALS"

// ---------------------------------------------------------------------------
/** @brief Set whether the fbx importer will read materials.
 *
 * The default value is true (1)
 * Property type: bool
 */
#define AI_CONFIG_IMPORT_FBX_READ_MATERIALS \
    "IMPORT_FBX_READ_MATERIALS"

// ---------------------------------------------------------------------------
/** @brief Set whether the fbx importer will read embedded textures.
 *
 * The default value is true (1)
 * Property type: bool
 */
#define AI_CONFIG_IMPORT_FBX_READ_TEXTURES \
    "IMPORT_FBX_READ_TEXTURES"

// ---------------------------------------------------------------------------
/** @brief Set whether the fbx importer will read cameras.
 *
 * The default value is true (1)
 * Property type: bool
 */
#define AI_CONFIG_IMPORT_FBX_READ_CAMERAS \
    "IMPORT_FBX_READ_CAMERAS"

// ---------------------------------------------------------------------------
/** @brief Set whether the fbx importer will read light sources.
 *
 * The default value is true (1)
 * Property type: bool
 */
#define AI_CONFIG_IMPORT_FBX_READ_LIGHTS \
    "IMPORT_FBX_READ_LIGHTS"

// ---------------------------------------------------------------------------
/** @brief Set whether the fbx importer will read animations.
 *
 * The default value is true (1)
 * Property type: bool
 */
#define AI_CONFIG_IMPORT_FBX_READ_ANIMATIONS \
    "IMPORT_FBX_READ_ANIMATIONS"

// ---------------------------------------------------------------------------
/** @brief Set whether the fbx importer will read weights.
 *
 * The default value is true (1)
 * Property type: bool
 */
#define AI_CONFIG_IMPORT_FBX_READ_WEIGHTS \
    "IMPORT_FBX_READ_WEIGHTS"

// ---------------------------------------------------------------------------
/** @brief Set whether the fbx importer will act in strict mode in which only
 *    FBX 2013 is supported and any other sub formats are rejected. FBX 2013
 *    is the primary target for the importer, so this format is best
 *    supported and well-tested.
 *
 * The default value is false (0)
 * Property type: bool
 */
#define AI_CONFIG_IMPORT_FBX_STRICT_MODE \
    "IMPORT_FBX_STRICT_MODE"

// ---------------------------------------------------------------------------
/** @brief Set whether the fbx importer will preserve pivot points for
 *    transformations (as extra nodes). If set to false, pivots and offsets
 *    will be evaluated whenever possible.
 *
 * The default value is true (1)
 * Property type: bool
 */
#define AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS \
    "IMPORT_FBX_PRESERVE_PIVOTS"

// ---------------------------------------------------------------------------
/** @brief Specifies whether the importer will drop empty animation curves or
 *    animation curves which match the bind pose transformation over their
 *    entire defined range.
 *
 * The default value is true (1)
 * Property type: bool
 */
#define AI_CONFIG_IMPORT_FBX_OPTIMIZE_EMPTY_ANIMATION_CURVES \
    "IMPORT_FBX_OPTIMIZE_EMPTY_ANIMATION_CURVES"

// ---------------------------------------------------------------------------
/** @brief Set whether the fbx importer will use the legacy embedded texture naming.
 *
 * The default value is false (0)
 * Property type: bool
 */
#define AI_CONFIG_IMPORT_FBX_EMBEDDED_TEXTURES_LEGACY_NAMING \
	"AI_CONFIG_IMPORT_FBX_EMBEDDED_TEXTURES_LEGACY_NAMING"

// ---------------------------------------------------------------------------
/** @brief  Set wether the importer shall not remove empty bones.
 *
 *  Empty bone are often used to define connections for other models.
 */
#define AI_CONFIG_IMPORT_REMOVE_EMPTY_BONES \
    "AI_CONFIG_IMPORT_REMOVE_EMPTY_BONES"


// ---------------------------------------------------------------------------
/** @brief  Set wether the FBX importer shall convert the unit from cm to m.
 */
#define AI_CONFIG_FBX_CONVERT_TO_M \
    "AI_CONFIG_FBX_CONVERT_TO_M"

// ---------------------------------------------------------------------------
/** @brief  Set the vertex animation keyframe to be imported
 *
 * ASSIMP does not support vertex keyframes (only bone animation is supported).
 * The library reads only one frame of models with vertex animations.
 * By default this is the first frame.
 * \note The default value is 0. This option applies to all importers.
 *   However, it is also possible to override the global setting
 *   for a specific loader. You can use the AI_CONFIG_IMPORT_XXX_KEYFRAME
 *   options (where XXX is a placeholder for the file format for which you
 *   want to override the global setting).
 * Property type: integer.
 */
#define AI_CONFIG_IMPORT_GLOBAL_KEYFRAME    "IMPORT_GLOBAL_KEYFRAME"

#define AI_CONFIG_IMPORT_MD3_KEYFRAME       "IMPORT_MD3_KEYFRAME"
#define AI_CONFIG_IMPORT_MD2_KEYFRAME       "IMPORT_MD2_KEYFRAME"
#define AI_CONFIG_IMPORT_MDL_KEYFRAME       "IMPORT_MDL_KEYFRAME"
#define AI_CONFIG_IMPORT_MDC_KEYFRAME       "IMPORT_MDC_KEYFRAME"
#define AI_CONFIG_IMPORT_SMD_KEYFRAME       "IMPORT_SMD_KEYFRAME"
#define AI_CONFIG_IMPORT_UNREAL_KEYFRAME    "IMPORT_UNREAL_KEYFRAME"

// ---------------------------------------------------------------------------
/** @brief Set whether the MDL (HL1) importer will read animations.
 *
 * The default value is true (1)
 * Property type: bool
 */
#define AI_CONFIG_IMPORT_MDL_HL1_READ_ANIMATIONS "IMPORT_MDL_HL1_READ_ANIMATIONS"

// ---------------------------------------------------------------------------
/** @brief Set whether the MDL (HL1) importer will read animation events.
 * \note This property requires AI_CONFIG_IMPORT_MDL_HL1_READ_ANIMATIONS to be set to true.
 *
 * The default value is true (1)
 * Property type: bool
 */
#define AI_CONFIG_IMPORT_MDL_HL1_READ_ANIMATION_EVENTS "IMPORT_MDL_HL1_READ_ANIMATION_EVENTS"

// ---------------------------------------------------------------------------
/** @brief Set whether the MDL (HL1) importer will read blend controllers.
 * \note This property requires AI_CONFIG_IMPORT_MDL_HL1_READ_ANIMATIONS to be set to true.
 *
 * The default value is true (1)
 * Property type: bool
 */
#define AI_CONFIG_IMPORT_MDL_HL1_READ_BLEND_CONTROLLERS "IMPORT_MDL_HL1_READ_BLEND_CONTROLLERS"

// ---------------------------------------------------------------------------
/** @brief Set whether the MDL (HL1) importer will read sequence transition graph.
 * \note This property requires AI_CONFIG_IMPORT_MDL_HL1_READ_ANIMATIONS to be set to true.
 *
 * The default value is true (1)
 * Property type: bool
 */
#define AI_CONFIG_IMPORT_MDL_HL1_READ_SEQUENCE_TRANSITIONS "IMPORT_MDL_HL1_READ_SEQUENCE_TRANSITIONS"

// ---------------------------------------------------------------------------
/** @brief Set whether the MDL (HL1) importer will read attachments info.
 *
 * The default value is true (1)
 * Property type: bool
 */
#define AI_CONFIG_IMPORT_MDL_HL1_READ_ATTACHMENTS "IMPORT_MDL_HL1_READ_ATTACHMENTS"

// ---------------------------------------------------------------------------
/** @brief Set whether the MDL (HL1) importer will read bone controllers info.
 *
 * The default value is true (1)
 * Property type: bool
 */
#define AI_CONFIG_IMPORT_MDL_HL1_READ_BONE_CONTROLLERS "IMPORT_MDL_HL1_READ_BONE_CONTROLLERS"

// ---------------------------------------------------------------------------
/** @brief Set whether the MDL (HL1) importer will read hitboxes info.
 *
 * The default value is true (1)
 * Property type: bool
 */
#define AI_CONFIG_IMPORT_MDL_HL1_READ_HITBOXES "IMPORT_MDL_HL1_READ_HITBOXES"

// ---------------------------------------------------------------------------
/** @brief Set whether the MDL (HL1) importer will read miscellaneous global model info.
 *
 * The default value is true (1)
 * Property type: bool
 */
#define AI_CONFIG_IMPORT_MDL_HL1_READ_MISC_GLOBAL_INFO "IMPORT_MDL_HL1_READ_MISC_GLOBAL_INFO"

// ---------------------------------------------------------------------------
/** Smd load multiple animations
 *
 *  Property type: bool. Default value: true.
 */
#define AI_CONFIG_IMPORT_SMD_LOAD_ANIMATION_LIST "IMPORT_SMD_LOAD_ANIMATION_LIST"

// ---------------------------------------------------------------------------
/** @brief  Configures the AC loader to collect all surfaces which have the
 *    "Backface cull" flag set in separate meshes.
 *
 *  Property type: bool. Default value: true.
 */
#define AI_CONFIG_IMPORT_AC_SEPARATE_BFCULL \
    "IMPORT_AC_SEPARATE_BFCULL"

// ---------------------------------------------------------------------------
/** @brief  Configures whether the AC loader evaluates subdivision surfaces (
 *  indicated by the presence of the 'subdiv' attribute in the file). By
 *  default, Assimp performs the subdivision using the standard
 *  Catmull-Clark algorithm
 *
 * * Property type: bool. Default value: true.
 */
#define AI_CONFIG_IMPORT_AC_EVAL_SUBDIVISION    \
    "IMPORT_AC_EVAL_SUBDIVISION"

// ---------------------------------------------------------------------------
/** @brief  Configures the UNREAL 3D loader to separate faces with different
 *    surface flags (e.g. two-sided vs. single-sided).
 *
 * * Property type: bool. Default value: true.
 */
#define AI_CONFIG_IMPORT_UNREAL_HANDLE_FLAGS \
    "UNREAL_HANDLE_FLAGS"

// ---------------------------------------------------------------------------
/** @brief Configures the terragen import plugin to compute uv's for
 *  terrains, if not given. Furthermore a default texture is assigned.
 *
 * UV coordinates for terrains are so simple to compute that you'll usually
 * want to compute them on your own, if you need them. This option is intended
 * for model viewers which want to offer an easy way to apply textures to
 * terrains.
 * * Property type: bool. Default value: false.
 */
#define AI_CONFIG_IMPORT_TER_MAKE_UVS \
    "IMPORT_TER_MAKE_UVS"

// ---------------------------------------------------------------------------
/** @brief  Configures the ASE loader to always reconstruct normal vectors
 *  basing on the smoothing groups loaded from the file.
 *
 * Some ASE files have carry invalid normals, other don't.
 * * Property type: bool. Default value: true.
 */
#define AI_CONFIG_IMPORT_ASE_RECONSTRUCT_NORMALS    \
    "IMPORT_ASE_RECONSTRUCT_NORMALS"

// ---------------------------------------------------------------------------
/** @brief  Configures the M3D loader to detect and process multi-part
 *    Quake player models.
 *
 * These models usually consist of 3 files, lower.md3, upper.md3 and
 * head.md3. If this property is set to true, Assimp will try to load and
 * combine all three files if one of them is loaded.
 * Property type: bool. Default value: true.
 */
#define AI_CONFIG_IMPORT_MD3_HANDLE_MULTIPART \
    "IMPORT_MD3_HANDLE_MULTIPART"

// ---------------------------------------------------------------------------
/** @brief  Tells the MD3 loader which skin files to load.
 *
 * When loading MD3 files, Assimp checks whether a file
 * [md3_file_name]_[skin_name].skin is existing. These files are used by
 * Quake III to be able to assign different skins (e.g. red and blue team)
 * to models. 'default', 'red', 'blue' are typical skin names.
 * Property type: String. Default value: "default".
 */
#define AI_CONFIG_IMPORT_MD3_SKIN_NAME \
    "IMPORT_MD3_SKIN_NAME"

// ---------------------------------------------------------------------------
/** @brief  Specify if to try load Quake 3 shader files. This also controls
 *  original surface name handling: when disabled it will be used unchanged.
 *
 * Property type: bool. Default value: true.
 */
#define AI_CONFIG_IMPORT_MD3_LOAD_SHADERS \
    "IMPORT_MD3_LOAD_SHADERS"

// ---------------------------------------------------------------------------
/** @brief  Specify the Quake 3 shader file to be used for a particular
 *  MD3 file. This can also be a search path.
 *
 * By default Assimp's behaviour is as follows: If a MD3 file
 * <tt>any_path/models/any_q3_subdir/model_name/file_name.md3</tt> is
 * loaded, the library tries to locate the corresponding shader file in
 * <tt>any_path/scripts/model_name.shader</tt>. This property overrides this
 * behaviour. It can either specify a full path to the shader to be loaded
 * or alternatively the path (relative or absolute) to the directory where
 * the shaders for all MD3s to be loaded reside. Assimp attempts to open
 * <tt>IMPORT_MD3_SHADER_SRC/model_name.shader</tt> first, <tt>IMPORT_MD3_SHADER_SRC/file_name.shader</tt>
 * is the fallback file. Note that IMPORT_MD3_SHADER_SRC should have a terminal (back)slash.
 * Property type: String. Default value: n/a.
 */
#define AI_CONFIG_IMPORT_MD3_SHADER_SRC \
    "IMPORT_MD3_SHADER_SRC"

// ---------------------------------------------------------------------------
/** @brief  Configures the LWO loader to load just one layer from the model.
 *
 * LWO files consist of layers and in some cases it could be useful to load
 * only one of them. This property can be either a string - which specifies
 * the name of the layer - or an integer - the index of the layer. If the
 * property is not set the whole LWO model is loaded. Loading fails if the
 * requested layer is not available. The layer index is zero-based and the
 * layer name may not be empty.<br>
 * Property type: Integer. Default value: all layers are loaded.
 */
#define AI_CONFIG_IMPORT_LWO_ONE_LAYER_ONLY         \
    "IMPORT_LWO_ONE_LAYER_ONLY"

// ---------------------------------------------------------------------------
/** @brief  Configures the MD5 loader to not load the MD5ANIM file for
 *  a MD5MESH file automatically.
 *
 * The default strategy is to look for a file with the same name but the
 * MD5ANIM extension in the same directory. If it is found, it is loaded
 * and combined with the MD5MESH file. This configuration option can be
 * used to disable this behaviour.
 *
 * * Property type: bool. Default value: false.
 */
#define AI_CONFIG_IMPORT_MD5_NO_ANIM_AUTOLOAD           \
    "IMPORT_MD5_NO_ANIM_AUTOLOAD"

// ---------------------------------------------------------------------------
/** @brief Defines the begin of the time range for which the LWS loader
 *    evaluates animations and computes aiNodeAnim's.
 *
 * Assimp provides full conversion of LightWave's envelope system, including
 * pre and post conditions. The loader computes linearly subsampled animation
 * chanels with the frame rate given in the LWS file. This property defines
 * the start time. Note: animation channels are only generated if a node
 * has at least one envelope with more tan one key assigned. This property.
 * is given in frames, '0' is the first frame. By default, if this property
 * is not set, the importer takes the animation start from the input LWS
 * file ('FirstFrame' line)<br>
 * Property type: Integer. Default value: taken from file.
 *
 * @see AI_CONFIG_IMPORT_LWS_ANIM_END - end of the imported time range
 */
#define AI_CONFIG_IMPORT_LWS_ANIM_START         \
    "IMPORT_LWS_ANIM_START"
#define AI_CONFIG_IMPORT_LWS_ANIM_END           \
    "IMPORT_LWS_ANIM_END"

// ---------------------------------------------------------------------------
/** @brief Defines the output frame rate of the IRR loader.
 *
 * IRR animations are difficult to convert for Assimp and there will
 * always be a loss of quality. This setting defines how many keys per second
 * are returned by the converter.<br>
 * Property type: integer. Default value: 100
 */
#define AI_CONFIG_IMPORT_IRR_ANIM_FPS               \
    "IMPORT_IRR_ANIM_FPS"

// ---------------------------------------------------------------------------
/** @brief Ogre Importer will try to find referenced materials from this file.
 *
 * Ogre meshes reference with material names, this does not tell Assimp the file
 * where it is located in. Assimp will try to find the source file in the following
 * order: <material-name>.material, <mesh-filename-base>.material and
 * lastly the material name defined by this config property.
 * <br>
 * Property type: String. Default value: Scene.material.
 */
#define AI_CONFIG_IMPORT_OGRE_MATERIAL_FILE \
    "IMPORT_OGRE_MATERIAL_FILE"

// ---------------------------------------------------------------------------
/** @brief Ogre Importer detect the texture usage from its filename.
 *
 * Ogre material texture units do not define texture type, the textures usage
 * depends on the used shader or Ogre's fixed pipeline. If this config property
 * is true Assimp will try to detect the type from the textures filename postfix:
 * _n, _nrm, _nrml, _normal, _normals and _normalmap for normal map, _s, _spec,
 * _specular and _specularmap for specular map, _l, _light, _lightmap, _occ
 * and _occlusion for light map, _disp and _displacement for displacement map.
 * The matching is case insensitive. Post fix is taken between the last
 * underscore and the last period.
 * Default behavior is to detect type from lower cased texture unit name by
 * matching against: normalmap, specularmap, lightmap and displacementmap.
 * For both cases if no match is found aiTextureType_DIFFUSE is used.
 * <br>
 * Property type: Bool. Default value: false.
 */
#define AI_CONFIG_IMPORT_OGRE_TEXTURETYPE_FROM_FILENAME \
    "IMPORT_OGRE_TEXTURETYPE_FROM_FILENAME"

 /** @brief Specifies whether the Android JNI asset extraction is supported.
  *
  * Turn on this option if you want to manage assets in native
  * Android application without having to keep the internal directory and asset
  * manager pointer.
  */
 #define AI_CONFIG_ANDROID_JNI_ASSIMP_MANAGER_SUPPORT "AI_CONFIG_ANDROID_JNI_ASSIMP_MANAGER_SUPPORT"

// ---------------------------------------------------------------------------
/** @brief Specifies whether the IFC loader skips over IfcSpace elements.
 *
 * IfcSpace elements (and their geometric representations) are used to
 * represent, well, free space in a building storey.<br>
 * Property type: Bool. Default value: true.
 */
#define AI_CONFIG_IMPORT_IFC_SKIP_SPACE_REPRESENTATIONS "IMPORT_IFC_SKIP_SPACE_REPRESENTATIONS"

// ---------------------------------------------------------------------------
/** @brief Specifies whether the IFC loader will use its own, custom triangulation
 *   algorithm to triangulate wall and floor meshes.
 *
 * If this property is set to false, walls will be either triangulated by
 * #aiProcess_Triangulate or will be passed through as huge polygons with
 * faked holes (i.e. holes that are connected with the outer boundary using
 * a dummy edge). It is highly recommended to set this property to true
 * if you want triangulated data because #aiProcess_Triangulate is known to
 * have problems with the kind of polygons that the IFC loader spits out for
 * complicated meshes.
 * Property type: Bool. Default value: true.
 */
#define AI_CONFIG_IMPORT_IFC_CUSTOM_TRIANGULATION "IMPORT_IFC_CUSTOM_TRIANGULATION"

// ---------------------------------------------------------------------------
/** @brief  Set the tessellation conic angle for IFC smoothing curves.
 *
 * This is used by the IFC importer to determine the tessellation parameter
 * for smoothing curves.
 * @note The default value is AI_IMPORT_IFC_DEFAULT_SMOOTHING_ANGLE and the
 * accepted values are in range [5.0, 120.0].
 * Property type: Float.
 */
#define AI_CONFIG_IMPORT_IFC_SMOOTHING_ANGLE "IMPORT_IFC_SMOOTHING_ANGLE"

// default value for AI_CONFIG_IMPORT_IFC_SMOOTHING_ANGLE
#if (!defined AI_IMPORT_IFC_DEFAULT_SMOOTHING_ANGLE)
#   define AI_IMPORT_IFC_DEFAULT_SMOOTHING_ANGLE 10.0f
#endif

// ---------------------------------------------------------------------------
/** @brief  Set the tessellation for IFC cylindrical shapes.
 *
 * This is used by the IFC importer to determine the tessellation parameter
 * for cylindrical shapes, i.e. the number of segments used to approximate a circle.
 * @note The default value is AI_IMPORT_IFC_DEFAULT_CYLINDRICAL_TESSELLATION and the
 * accepted values are in range [3, 180].
 * Property type: Integer.
 */
#define AI_CONFIG_IMPORT_IFC_CYLINDRICAL_TESSELLATION "IMPORT_IFC_CYLINDRICAL_TESSELLATION"

// default value for AI_CONFIG_IMPORT_IFC_CYLINDRICAL_TESSELLATION
#if (!defined AI_IMPORT_IFC_DEFAULT_CYLINDRICAL_TESSELLATION)
#   define AI_IMPORT_IFC_DEFAULT_CYLINDRICAL_TESSELLATION 32
#endif

// ---------------------------------------------------------------------------
/** @brief Specifies whether the Collada loader will ignore the provided up direction.
 *
 * If this property is set to true, the up direction provided in the file header will
 * be ignored and the file will be loaded as is.
 * Property type: Bool. Default value: false.
 */
#define AI_CONFIG_IMPORT_COLLADA_IGNORE_UP_DIRECTION "IMPORT_COLLADA_IGNORE_UP_DIRECTION"

// ---------------------------------------------------------------------------
/** @brief Specifies whether the Collada loader should use Collada names.
 *
 * If this property is set to true, the Collada names will be used as the node and
 * mesh names. The default is to use the id tag (resp. sid tag, if no id tag is present)
 * instead.
 * Property type: Bool. Default value: false.
 */
#define AI_CONFIG_IMPORT_COLLADA_USE_COLLADA_NAMES "IMPORT_COLLADA_USE_COLLADA_NAMES"

// ---------- All the Export defines ------------

/** @brief Specifies the xfile use double for real values of float
 *
 * Property type: Bool. Default value: false.
 */

#define AI_CONFIG_EXPORT_XFILE_64BIT "EXPORT_XFILE_64BIT"

/** @brief Specifies whether the assimp export shall be able to export point clouds
 *
 *  When this flag is not defined the render data has to contain valid faces.
 *  Point clouds are only a collection of vertices which have nor spatial organization
 *  by a face and the validation process will remove them. Enabling this feature will
 *  switch off the flag and enable the functionality to export pure point clouds.
 */
#define AI_CONFIG_EXPORT_POINT_CLOUDS "EXPORT_POINT_CLOUDS"

/**
 * @brief Specifies the blob name, assimp uses for exporting.
 * 
 * Some formats require auxiliary files to be written, that need to be linked back into 
 * the original file. For example, OBJ files export materials to a separate MTL file and
 * use the `mtllib` keyword to reference this file.
 * 
 * When exporting blobs using #ExportToBlob, assimp does not know the name of the blob
 * file and thus outputs `mtllib $blobfile.mtl`, which might not be desired, since the 
 * MTL file might be called differently. 
 * 
 * This property can be used to give the exporter a hint on how to use the magic 
 * `$blobfile` keyword. If the exporter detects the keyword and is provided with a name
 * for the blob, it instead uses this name.
 */
#define AI_CONFIG_EXPORT_BLOB_NAME "EXPORT_BLOB_NAME"

/**
 *  @brief  Specifies a gobal key factor for scale, float value
 */
#define AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY "GLOBAL_SCALE_FACTOR"

#if (!defined AI_CONFIG_GLOBAL_SCALE_FACTOR_DEFAULT)
#   define AI_CONFIG_GLOBAL_SCALE_FACTOR_DEFAULT  1.0f
#endif // !! AI_DEBONE_THRESHOLD

#define AI_CONFIG_APP_SCALE_KEY "APP_SCALE_FACTOR"

#if (!defined AI_CONFIG_APP_SCALE_KEY)
#   define AI_CONFIG_APP_SCALE_KEY 1.0
#endif // AI_CONFIG_APP_SCALE_KEY


// ---------- All the Build/Compile-time defines ------------

/** @brief Specifies if double precision is supported inside assimp
 *
 * Property type: Bool. Default value: undefined.
 */

/* #undef ASSIMP_DOUBLE_PRECISION */

#endif // !! AI_CONFIG_H_INC
]])
renameAndDeleteFile("config.h", folderDirAssimp .. "include/assimp/config.h")
end

function m.createZconf()
    p.w([[
        /* zconf.h -- configuration of the zlib compression library
        * Copyright (C) 1995-2016 Jean-loup Gailly, Mark Adler
        * For conditions of distribution and use, see copyright notice in zlib.h
        */
       
       /* @(#) $Id$ */
       
       #ifndef ZCONF_H
       #define ZCONF_H
       /* #undef Z_PREFIX */
       /* #undef Z_HAVE_UNISTD_H */
       
       /*
        * If you *really* need a unique prefix for all types and library functions,
        * compile with -DZ_PREFIX. The "standard" zlib should be compiled without it.
        * Even better than compiling with -DZ_PREFIX would be to use configure to set
        * this permanently in zconf.h using "./configure --zprefix".
        */
       #ifdef Z_PREFIX     /* may be set to #if 1 by ./configure */
       #  define Z_PREFIX_SET
       
       /* all linked symbols and init macros */
       #  define _dist_code            z__dist_code
       #  define _length_code          z__length_code
       #  define _tr_align             z__tr_align
       #  define _tr_flush_bits        z__tr_flush_bits
       #  define _tr_flush_block       z__tr_flush_block
       #  define _tr_init              z__tr_init
       #  define _tr_stored_block      z__tr_stored_block
       #  define _tr_tally             z__tr_tally
       #  define adler32               z_adler32
       #  define adler32_combine       z_adler32_combine
       #  define adler32_combine64     z_adler32_combine64
       #  define adler32_z             z_adler32_z
       #  ifndef Z_SOLO
       #    define compress              z_compress
       #    define compress2             z_compress2
       #    define compressBound         z_compressBound
       #  endif
       #  define crc32                 z_crc32
       #  define crc32_combine         z_crc32_combine
       #  define crc32_combine64       z_crc32_combine64
       #  define crc32_z               z_crc32_z
       #  define deflate               z_deflate
       #  define deflateBound          z_deflateBound
       #  define deflateCopy           z_deflateCopy
       #  define deflateEnd            z_deflateEnd
       #  define deflateGetDictionary  z_deflateGetDictionary
       #  define deflateInit           z_deflateInit
       #  define deflateInit2          z_deflateInit2
       #  define deflateInit2_         z_deflateInit2_
       #  define deflateInit_          z_deflateInit_
       #  define deflateParams         z_deflateParams
       #  define deflatePending        z_deflatePending
       #  define deflatePrime          z_deflatePrime
       #  define deflateReset          z_deflateReset
       #  define deflateResetKeep      z_deflateResetKeep
       #  define deflateSetDictionary  z_deflateSetDictionary
       #  define deflateSetHeader      z_deflateSetHeader
       #  define deflateTune           z_deflateTune
       #  define deflate_copyright     z_deflate_copyright
       #  define get_crc_table         z_get_crc_table
       #  ifndef Z_SOLO
       #    define gz_error              z_gz_error
       #    define gz_intmax             z_gz_intmax
       #    define gz_strwinerror        z_gz_strwinerror
       #    define gzbuffer              z_gzbuffer
       #    define gzclearerr            z_gzclearerr
       #    define gzclose               z_gzclose
       #    define gzclose_r             z_gzclose_r
       #    define gzclose_w             z_gzclose_w
       #    define gzdirect              z_gzdirect
       #    define gzdopen               z_gzdopen
       #    define gzeof                 z_gzeof
       #    define gzerror               z_gzerror
       #    define gzflush               z_gzflush
       #    define gzfread               z_gzfread
       #    define gzfwrite              z_gzfwrite
       #    define gzgetc                z_gzgetc
       #    define gzgetc_               z_gzgetc_
       #    define gzgets                z_gzgets
       #    define gzoffset              z_gzoffset
       #    define gzoffset64            z_gzoffset64
       #    define gzopen                z_gzopen
       #    define gzopen64              z_gzopen64
       #    ifdef _WIN32
       #      define gzopen_w              z_gzopen_w
       #    endif
       #    define gzprintf              z_gzprintf
       #    define gzputc                z_gzputc
       #    define gzputs                z_gzputs
       #    define gzread                z_gzread
       #    define gzrewind              z_gzrewind
       #    define gzseek                z_gzseek
       #    define gzseek64              z_gzseek64
       #    define gzsetparams           z_gzsetparams
       #    define gztell                z_gztell
       #    define gztell64              z_gztell64
       #    define gzungetc              z_gzungetc
       #    define gzvprintf             z_gzvprintf
       #    define gzwrite               z_gzwrite
       #  endif
       #  define inflate               z_inflate
       #  define inflateBack           z_inflateBack
       #  define inflateBackEnd        z_inflateBackEnd
       #  define inflateBackInit       z_inflateBackInit
       #  define inflateBackInit_      z_inflateBackInit_
       #  define inflateCodesUsed      z_inflateCodesUsed
       #  define inflateCopy           z_inflateCopy
       #  define inflateEnd            z_inflateEnd
       #  define inflateGetDictionary  z_inflateGetDictionary
       #  define inflateGetHeader      z_inflateGetHeader
       #  define inflateInit           z_inflateInit
       #  define inflateInit2          z_inflateInit2
       #  define inflateInit2_         z_inflateInit2_
       #  define inflateInit_          z_inflateInit_
       #  define inflateMark           z_inflateMark
       #  define inflatePrime          z_inflatePrime
       #  define inflateReset          z_inflateReset
       #  define inflateReset2         z_inflateReset2
       #  define inflateResetKeep      z_inflateResetKeep
       #  define inflateSetDictionary  z_inflateSetDictionary
       #  define inflateSync           z_inflateSync
       #  define inflateSyncPoint      z_inflateSyncPoint
       #  define inflateUndermine      z_inflateUndermine
       #  define inflateValidate       z_inflateValidate
       #  define inflate_copyright     z_inflate_copyright
       #  define inflate_fast          z_inflate_fast
       #  define inflate_table         z_inflate_table
       #  ifndef Z_SOLO
       #    define uncompress            z_uncompress
       #    define uncompress2           z_uncompress2
       #  endif
       #  define zError                z_zError
       #  ifndef Z_SOLO
       #    define zcalloc               z_zcalloc
       #    define zcfree                z_zcfree
       #  endif
       #  define zlibCompileFlags      z_zlibCompileFlags
       #  define zlibVersion           z_zlibVersion
       
       /* all zlib typedefs in zlib.h and zconf.h */
       #  define Byte                  z_Byte
       #  define Bytef                 z_Bytef
       #  define alloc_func            z_alloc_func
       #  define charf                 z_charf
       #  define free_func             z_free_func
       #  ifndef Z_SOLO
       #    define gzFile                z_gzFile
       #  endif
       #  define gz_header             z_gz_header
       #  define gz_headerp            z_gz_headerp
       #  define in_func               z_in_func
       #  define intf                  z_intf
       #  define out_func              z_out_func
       #  define uInt                  z_uInt
       #  define uIntf                 z_uIntf
       #  define uLong                 z_uLong
       #  define uLongf                z_uLongf
       #  define voidp                 z_voidp
       #  define voidpc                z_voidpc
       #  define voidpf                z_voidpf
       
       /* all zlib structs in zlib.h and zconf.h */
       #  define gz_header_s           z_gz_header_s
       #  define internal_state        z_internal_state
       
       #endif
       
       #if defined(__MSDOS__) && !defined(MSDOS)
       #  define MSDOS
       #endif
       #if (defined(OS_2) || defined(__OS2__)) && !defined(OS2)
       #  define OS2
       #endif
       #if defined(_WINDOWS) && !defined(WINDOWS)
       #  define WINDOWS
       #endif
       #if defined(_WIN32) || defined(_WIN32_WCE) || defined(__WIN32__)
       #  ifndef WIN32
       #    define WIN32
       #  endif
       #endif
       #if (defined(MSDOS) || defined(OS2) || defined(WINDOWS)) && !defined(WIN32)
       #  if !defined(__GNUC__) && !defined(__FLAT__) && !defined(__386__)
       #    ifndef SYS16BIT
       #      define SYS16BIT
       #    endif
       #  endif
       #endif
       
       /*
        * Compile with -DMAXSEG_64K if the alloc function cannot allocate more
        * than 64k bytes at a time (needed on systems with 16-bit int).
        */
       #ifdef SYS16BIT
       #  define MAXSEG_64K
       #endif
       #ifdef MSDOS
       #  define UNALIGNED_OK
       #endif
       
       #ifdef __STDC_VERSION__
       #  ifndef STDC
       #    define STDC
       #  endif
       #  if __STDC_VERSION__ >= 199901L
       #    ifndef STDC99
       #      define STDC99
       #    endif
       #  endif
       #endif
       #if !defined(STDC) && (defined(__STDC__) || defined(__cplusplus))
       #  define STDC
       #endif
       #if !defined(STDC) && (defined(__GNUC__) || defined(__BORLANDC__))
       #  define STDC
       #endif
       #if !defined(STDC) && (defined(MSDOS) || defined(WINDOWS) || defined(WIN32))
       #  define STDC
       #endif
       #if !defined(STDC) && (defined(OS2) || defined(__HOS_AIX__))
       #  define STDC
       #endif
       
       #if defined(__OS400__) && !defined(STDC)    /* iSeries (formerly AS/400). */
       #  define STDC
       #endif
       
       #ifndef STDC
       #  ifndef const /* cannot use !defined(STDC) && !defined(const) on Mac */
       #    define const       /* note: need a more gentle solution here */
       #  endif
       #endif
       
       #if defined(ZLIB_CONST) && !defined(z_const)
       #  define z_const const
       #else
       #  define z_const
       #endif
       
       #ifdef Z_SOLO
          typedef unsigned long z_size_t;
       #else
       #  define z_longlong long long
       #  if defined(NO_SIZE_T)
            typedef unsigned NO_SIZE_T z_size_t;
       #  elif defined(STDC)
       #    include <stddef.h>
            typedef size_t z_size_t;
       #  else
            typedef unsigned long z_size_t;
       #  endif
       #  undef z_longlong
       #endif
       
       /* Maximum value for memLevel in deflateInit2 */
       #ifndef MAX_MEM_LEVEL
       #  ifdef MAXSEG_64K
       #    define MAX_MEM_LEVEL 8
       #  else
       #    define MAX_MEM_LEVEL 9
       #  endif
       #endif
       
       /* Maximum value for windowBits in deflateInit2 and inflateInit2.
        * WARNING: reducing MAX_WBITS makes minigzip unable to extract .gz files
        * created by gzip. (Files created by minigzip can still be extracted by
        * gzip.)
        */
       #ifndef MAX_WBITS
       #  define MAX_WBITS   15 /* 32K LZ77 window */
       #endif
       
       /* The memory requirements for deflate are (in bytes):
                   (1 << (windowBits+2)) +  (1 << (memLevel+9))
        that is: 128K for windowBits=15  +  128K for memLevel = 8  (default values)
        plus a few kilobytes for small objects. For example, if you want to reduce
        the default memory requirements from 256K to 128K, compile with
            make CFLAGS="-O -DMAX_WBITS=14 -DMAX_MEM_LEVEL=7"
        Of course this will generally degrade compression (there's no free lunch).
       
          The memory requirements for inflate are (in bytes) 1 << windowBits
        that is, 32K for windowBits=15 (default value) plus about 7 kilobytes
        for small objects.
       */
       
                               /* Type declarations */
       
       #ifndef OF /* function prototypes */
       #  ifdef STDC
       #    define OF(args)  args
       #  else
       #    define OF(args)  ()
       #  endif
       #endif
       
       #ifndef Z_ARG /* function prototypes for stdarg */
       #  if defined(STDC) || defined(Z_HAVE_STDARG_H)
       #    define Z_ARG(args)  args
       #  else
       #    define Z_ARG(args)  ()
       #  endif
       #endif
       
       /* The following definitions for FAR are needed only for MSDOS mixed
        * model programming (small or medium model with some far allocations).
        * This was tested only with MSC; for other MSDOS compilers you may have
        * to define NO_MEMCPY in zutil.h.  If you don't need the mixed model,
        * just define FAR to be empty.
        */
       #ifdef SYS16BIT
       #  if defined(M_I86SM) || defined(M_I86MM)
            /* MSC small or medium model */
       #    define SMALL_MEDIUM
       #    ifdef _MSC_VER
       #      define FAR _far
       #    else
       #      define FAR far
       #    endif
       #  endif
       #  if (defined(__SMALL__) || defined(__MEDIUM__))
            /* Turbo C small or medium model */
       #    define SMALL_MEDIUM
       #    ifdef __BORLANDC__
       #      define FAR _far
       #    else
       #      define FAR far
       #    endif
       #  endif
       #endif
       
       #if defined(WINDOWS) || defined(WIN32)
          /* If building or using zlib as a DLL, define ZLIB_DLL.
           * This is not mandatory, but it offers a little performance increase.
           */
       #  ifdef ZLIB_DLL
       #    if defined(WIN32) && (!defined(__BORLANDC__) || (__BORLANDC__ >= 0x500))
       #      ifdef ZLIB_INTERNAL
       #        define ZEXTERN extern __declspec(dllexport)
       #      else
       #        define ZEXTERN extern __declspec(dllimport)
       #      endif
       #    endif
       #  endif  /* ZLIB_DLL */
          /* If building or using zlib with the WINAPI/WINAPIV calling convention,
           * define ZLIB_WINAPI.
           * Caution: the standard ZLIB1.DLL is NOT compiled using ZLIB_WINAPI.
           */
       #  ifdef ZLIB_WINAPI
       #    ifdef FAR
       #      undef FAR
       #    endif
       #    include <windows.h>
            /* No need for _export, use ZLIB.DEF instead. */
            /* For complete Windows compatibility, use WINAPI, not __stdcall. */
       #    define ZEXPORT WINAPI
       #    ifdef WIN32
       #      define ZEXPORTVA WINAPIV
       #    else
       #      define ZEXPORTVA FAR CDECL
       #    endif
       #  endif
       #endif
       
       #if defined (__BEOS__)
       #  ifdef ZLIB_DLL
       #    ifdef ZLIB_INTERNAL
       #      define ZEXPORT   __declspec(dllexport)
       #      define ZEXPORTVA __declspec(dllexport)
       #    else
       #      define ZEXPORT   __declspec(dllimport)
       #      define ZEXPORTVA __declspec(dllimport)
       #    endif
       #  endif
       #endif
       
       #ifndef ZEXTERN
       #  define ZEXTERN extern
       #endif
       #ifndef ZEXPORT
       #  define ZEXPORT
       #endif
       #ifndef ZEXPORTVA
       #  define ZEXPORTVA
       #endif
       
       #ifndef FAR
       #  define FAR
       #endif
       
       #if !defined(__MACTYPES__)
       typedef unsigned char  Byte;  /* 8 bits */
       #endif
       typedef unsigned int   uInt;  /* 16 bits or more */
       typedef unsigned long  uLong; /* 32 bits or more */
       
       #ifdef SMALL_MEDIUM
          /* Borland C/C++ and some old MSC versions ignore FAR inside typedef */
       #  define Bytef Byte FAR
       #else
          typedef Byte  FAR Bytef;
       #endif
       typedef char  FAR charf;
       typedef int   FAR intf;
       typedef uInt  FAR uIntf;
       typedef uLong FAR uLongf;
       
       #ifdef STDC
          typedef void const *voidpc;
          typedef void FAR   *voidpf;
          typedef void       *voidp;
       #else
          typedef Byte const *voidpc;
          typedef Byte FAR   *voidpf;
          typedef Byte       *voidp;
       #endif
       
       #if !defined(Z_U4) && !defined(Z_SOLO) && defined(STDC)
       #  include <limits.h>
       #  if (UINT_MAX == 0xffffffffUL)
       #    define Z_U4 unsigned
       #  elif (ULONG_MAX == 0xffffffffUL)
       #    define Z_U4 unsigned long
       #  elif (USHRT_MAX == 0xffffffffUL)
       #    define Z_U4 unsigned short
       #  endif
       #endif
       
       #ifdef Z_U4
          typedef Z_U4 z_crc_t;
       #else
          typedef unsigned long z_crc_t;
       #endif
       
       #ifdef HAVE_UNISTD_H    /* may be set to #if 1 by ./configure */
       #  define Z_HAVE_UNISTD_H
       #endif
       
       #ifdef HAVE_STDARG_H    /* may be set to #if 1 by ./configure */
       #  define Z_HAVE_STDARG_H
       #endif
       
       #ifdef STDC
       #  ifndef Z_SOLO
       #    include <sys/types.h>      /* for off_t */
       #  endif
       #endif
       
       #if defined(STDC) || defined(Z_HAVE_STDARG_H)
       #  ifndef Z_SOLO
       #    include <stdarg.h>         /* for va_list */
       #  endif
       #endif
       
       #ifdef _WIN32
       #  ifndef Z_SOLO
       #    include <stddef.h>         /* for wchar_t */
       #  endif
       #endif
       
       /* a little trick to accommodate both "#define _LARGEFILE64_SOURCE" and
        * "#define _LARGEFILE64_SOURCE 1" as requesting 64-bit operations, (even
        * though the former does not conform to the LFS document), but considering
        * both "#undef _LARGEFILE64_SOURCE" and "#define _LARGEFILE64_SOURCE 0" as
        * equivalently requesting no 64-bit operations
        */
       #if defined(_LARGEFILE64_SOURCE) && -_LARGEFILE64_SOURCE - -1 == 1
       #  undef _LARGEFILE64_SOURCE
       #endif
       
       #if defined(__WATCOMC__) && !defined(Z_HAVE_UNISTD_H)
       #  define Z_HAVE_UNISTD_H
       #endif
       #ifndef Z_SOLO
       #  if defined(Z_HAVE_UNISTD_H) || defined(_LARGEFILE64_SOURCE)
       #    include <unistd.h>         /* for SEEK_*, off_t, and _LFS64_LARGEFILE */
       #    ifdef VMS
       #      include <unixio.h>       /* for off_t */
       #    endif
       #    ifndef z_off_t
       #      define z_off_t off_t
       #    endif
       #  endif
       #endif
       
       #if defined(_LFS64_LARGEFILE) && _LFS64_LARGEFILE-0
       #  define Z_LFS64
       #endif
       
       #if defined(_LARGEFILE64_SOURCE) && defined(Z_LFS64)
       #  define Z_LARGE64
       #endif
       
       #if defined(_FILE_OFFSET_BITS) && _FILE_OFFSET_BITS-0 == 64 && defined(Z_LFS64)
       #  define Z_WANT64
       #endif
       
       #if !defined(SEEK_SET) && !defined(Z_SOLO)
       #  define SEEK_SET        0       /* Seek from beginning of file.  */
       #  define SEEK_CUR        1       /* Seek from current position.  */
       #  define SEEK_END        2       /* Set file pointer to EOF plus "offset" */
       #endif
       
       #ifndef z_off_t
       #  define z_off_t long
       #endif
       
       #if !defined(_WIN32) && defined(Z_LARGE64)
       #  define z_off64_t off64_t
       #else
       #  if defined(_WIN32) && !defined(__GNUC__) && !defined(Z_SOLO)
       #    define z_off64_t __int64
       #  else
       #    define z_off64_t z_off_t
       #  endif
       #endif
       
       /* MVS linker does not support external names larger than 8 bytes */
       #if defined(__MVS__)
         #pragma map(deflateInit_,"DEIN")
         #pragma map(deflateInit2_,"DEIN2")
         #pragma map(deflateEnd,"DEEND")
         #pragma map(deflateBound,"DEBND")
         #pragma map(inflateInit_,"ININ")
         #pragma map(inflateInit2_,"ININ2")
         #pragma map(inflateEnd,"INEND")
         #pragma map(inflateSync,"INSY")
         #pragma map(inflateSetDictionary,"INSEDI")
         #pragma map(compressBound,"CMBND")
         #pragma map(inflate_table,"INTABL")
         #pragma map(inflate_fast,"INFA")
         #pragma map(inflate_copyright,"INCOPY")
       #endif
       
       #endif /* ZCONF_H */
    ]])
    renameAndDeleteFile("zconf.h", folderDirAssimp .. "contrib/zlib/zconf.h")
end

function m.createRevision()
    p.w([[
        #ifndef ASSIMP_REVISION_H_INC
        #define ASSIMP_REVISION_H_INC
        
        #define GitVersion 0xf44a94e1
        #define GitBranch "master"
        
        #define VER_MAJOR 5
        #define VER_MINOR 2
        #define VER_PATCH 0
        #define VER_BUILD 0
        
        #define STR_HELP(x) #x
        #define STR(x) STR_HELP(x)
        
        #define VER_FILEVERSION             VER_MAJOR,VER_MINOR,VER_PATCH,VER_BUILD
        #if (GitVersion == 0)
        #define VER_FILEVERSION_STR         STR(VER_MAJOR) "." STR(VER_MINOR) "." STR(VER_PATCH) "." STR(VER_BUILD)
        #else
        #define VER_FILEVERSION_STR         STR(VER_MAJOR) "." STR(VER_MINOR) "." STR(VER_PATCH) "." STR(VER_BUILD) " (Commit f44a94e1)"
        #endif
        #define VER_COPYRIGHT_STR           "\xA9 2006-2022"
        
        #ifdef  NDEBUG
        #define VER_ORIGINAL_FILENAME_STR   "assimp.dll"
        #else
        #define VER_ORIGINAL_FILENAME_STR   "assimp.dll"
        #endif //  NDEBUG
        
        #endif // ASSIMP_REVISION_H_INC
]])
renameAndDeleteFile("revision.h", folderDirAssimp .. "include/revision.h")
end

p.generate("config", "config.h", m.createAssimpConfig)
p.generate("zconf", "zconf.h", m.createZconf)
p.generate("revision", "revision.h", m.createRevision)