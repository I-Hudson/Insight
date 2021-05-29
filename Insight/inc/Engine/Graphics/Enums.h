#pragma once

#include "Engine/Core/InsightAlias.h"
#include "Engine/Platform/Platform.h"

/// <summary>
/// Graphics rendering backend system types.
/// </summary>
enum class RendererType
{
    /// <summary>
    /// Unknown type
    /// </summary>
    Unknown = 0,

    /// <summary>
    /// DirectX 10
    /// </summary>
    DirectX10 = 1,

    /// <summary>
    /// DirectX 10.1
    /// </summary>
    DirectX10_1 = 2,

    /// <summary>
    /// DirectX 11
    /// </summary>
    DirectX11 = 3,

    /// <summary>
    /// DirectX 12
    /// </summary>
    DirectX12 = 4,

    /// <summary>
    /// OpenGL 4.1
    /// </summary>
    OpenGL4_1 = 5,

    /// <summary>
    /// OpenGL 4.4
    /// </summary>
    OpenGL4_4 = 6,

    /// <summary>
    /// OpenGL ES 3
    /// </summary>
    OpenGLES3 = 7,

    /// <summary>
    /// OpenGL ES 3.1
    /// </summary>
    OpenGLES3_1 = 8,

    /// <summary>
    /// Null backend
    /// </summary>
    Null = 9,

    /// <summary>
    /// Vulkan
    /// </summary>
    Vulkan = 10,

    /// <summary>
    /// PlayStation 4
    /// </summary>
    PS4 = 11,


    MAX
};

const char* ToString(RendererType value);

/// <summary>
/// Shader profile types define the version and type of the shading language used by the graphics backend.
/// </summary>
enum class ShaderProfile
{
    /// <summary>
    /// Unknown
    /// </summary>
    Unknown = 0,

    /// <summary>
    /// DirectX (Shader Model 4 compatible)
    /// </summary>
    DirectX_SM4 = 1,

    /// <summary>
    /// DirectX (Shader Model 5 compatible)
    /// </summary>
    DirectX_SM5 = 2,

    /// <summary>
    /// GLSL 410
    /// </summary>
    GLSL_410 = 3,

    /// <summary>
    /// GLSL 440
    /// </summary>
    GLSL_440 = 4,

    /// <summary>
    /// Vulkan (Shader Model 5 compatible)
    /// </summary>
    Vulkan_SM5 = 5,

    /// <summary>
    /// PlayStation 4
    /// </summary>
    PS4 = 6,

    /// <summary>
    /// DirectX (Shader Model 6 compatible)
    /// </summary>
    DirectX_SM6 = 7,


    MAX
};

const char* ToString(ShaderProfile value);

/// <summary>
/// Graphics feature levels indicates what level of support can be relied upon. 
/// They are named after the graphics API to indicate the minimum level of the features set to support. 
/// Feature levels are ordered from the lowest to the most high-end so feature level enum can be used to switch between feature levels (e.g. don't use geometry shader if not supported).
/// </summary>
enum class FeatureLevel
{
    /// <summary>
    /// The features set defined by the core capabilities of OpenGL ES2.
    /// </summary>
    ES2 = 0,

    /// <summary>
    /// The features set defined by the core capabilities of OpenGL ES3.
    /// </summary>
    ES3 = 1,

    /// <summary>
    /// The features set defined by the core capabilities of OpenGL ES3.1.
    /// </summary>
    ES3_1 = 2,

    /// <summary>
    /// The features set defined by the core capabilities of DirectX 10 Shader Model 4.
    /// </summary>
    SM4 = 3,

    /// <summary>
    /// The features set defined by the core capabilities of DirectX 11 Shader Model 5.
    /// </summary>
    SM5 = 4,

    /// <summary>
    /// The features set defined by the core capabilities of DirectX 12 Shader Model 6.
    /// </summary>
    SM6 = 5,


    MAX
};

enum class SampleLevel : i32
{
    None = 1,
    X2 = 2,
    X4 = 4,
    X8 = 8,
    X16 = 16,
    X32 = 32,
    X64 = 64,
};

enum class SamplerFilter
{
    Nearest = 0,
    Linear = 1,
    Cubic_Img = 2
};

enum class SamplerMipmapMode
{
    Nearest = 0,
    Linear = 1
};

enum class SamplerAddressMode
{
    Repeat = 0,
    Mirrored_Repeat = 1,
    Clamp_To_Edge = 2,
    Clamp_To_Border = 3,
    Mirror_Clamp_To_Edge = 4,
};

enum class CompareOp
{
    Never = 0,
    Less = 1,
    Equal = 2,
    Less_Or_Equal = 3,
    Greater = 4,
    Not_Equal = 5,
    Greater_Or_Equal = 6,
    Always = 7
};

enum class BorderColor
{
    Float_Transparent_Black = 0,
    Int_Transparent_Black = 1,
    Float_Opaque_Black = 2,
    Int_Opaque_Black = 3,
    Float_Opaque_White = 4,
    Int_Opaque_White = 5,

};

enum class PipelineBindPoint
{
    Graphics = 0,
    Compute = 1,
    Ray_Tracing = 2,
};

enum class PrimitiveTopologyType
{
    Point_List = 0,
    Line_List = 1,
    Line_Strip = 2,
    Triangle_List = 3,
    Triangle_Strip = 4,
    Triangle_Fan = 5,
    Line_List_With_Adjacency = 6,
    Line_Strip_With_Adjacency = 7,
    Triangle_List_With_Adjacency = 8,
    Triangle_Strip_With_Adjacency = 9,
    Patch_List = 10,
};

enum class PolygonMode
{
    Fill = 0,
    Line = 1,
    Point = 2,
};

enum class CullMode
{
    None = 0,
    Front = 1,
    Back = 2,
    Front_And_Back = 3,
};

enum class FrontFace
{
    Counter_Clockwise = 0,
    Clockwise = 1,
};

/// <summary>
/// Define the different stages a shader can have.
/// </summary>
enum class ShaderStage : u32
{
    Vertex,
    TessControl,
    TessEvaluation,
    Geometry,
    Fragment,
    Compute,

    Count
};

/// <summary>
/// Define the pipeline stages.
/// </summary>
enum class PipelineStage : u32
{
    Top_Of_Pipe             = 1 << 0,
    Draw_Indirect           = 1 << 0,
    Vertex_Input            = 1 << 1,
    Vertex_Shader           = 1 << 2,
    Tess_Control_Shader     = 1 << 3,
    Tess_Evaluation_Shader  = 1 << 4,
    Geometry_Shader         = 1 << 5,
    Fragment_Shader         = 1 << 6,
    Early_Fragment_Test     = 1 << 7,
    Late_Fragment_Test      = 1 << 8,
    Color_Attachment_Output = 1 << 9,
    Compute_Shader          = 1 << 10,
    Transfer                = 1 << 11,
    Bottom_Of_Pipe          = 1 << 12,
    Host                    = 1 << 13,
    All_Graphics            = 1 << 14,
    All_Commands            = 1 << 15,
};
using PipelineStageFlags = u32;
std::string PipelineStageFlagsToString(const PipelineStageFlags& flags);

/// <summary>
/// Define the image usage flags which a input within the 
/// rendering pipeline can be.
/// </summary>
enum ImageUsageFlagsBits
{
    Transfer_Src                = 1 << 0,
    Transfer_Dst                = 1 << 1,
    Sampled                     = 1 << 2,
    Storage                     = 1 << 3,
    Color_Attachment            = 1 << 4,
    Depth_Stencil_Attachment    = 1 << 5,
    Transient_Attachment        = 1 << 6,
    Input_Attachment            = 1 << 7
};
using ImageUsageFlags = u32;

enum class ImageMisc
{
    Generate_Mips                                   = 1 << 0,
    Force_Array                                     = 1 << 1,
    Mutable_SRGB                                    = 1 << 2,
    Concurrent_Queue_Graphics                       = 1 << 3,
    Concurrent_Queue_Async_Compute                  = 1 << 4,
    Concurrent_Queue_Async_Graphics                 = 1 << 5,
    Concurrent_Queue_Async_Transfer                 = 1 << 6,
    Verify_Format_Feature_Sampled_Linear_Filter     = 1 << 7,
    Linear_Image_Ignore_Device_Local                = 1 << 8,
    Force_No_Dedicated                              = 1 << 9,
};
using ImageMiscFlags = u32;

enum class ImageCreate
{
    Sparse_Binding = 1 << 0,
    Sparse_Residency = 1 << 1,
    Sparse_Aliased = 1 << 2,
    Mutable_Format = 1 << 3,
    Cube_Compatible = 1 << 4,
};
using ImageCreateFlags = u32;

enum class ImageLayout : u32
{
    Undefined                   = 1 << 0,
    General                     = 1 << 1,
    Color_Attachment            = 1 << 2,
    Depth_Stencil_Attachment    = 1 << 3,
    Depth_Stencil_Read_Only     = 1 << 4,
    Shader_Read_Only            = 1 << 5,
    Transfer_Src                = 1 << 6,
    Transfer_Dst                = 1 << 7,
    Preinitialized              = 1 << 8
};
using ImageLayoutFlags = u32;
const char* ImageLayoutToString(const ImageLayout& layout);

enum class ImageType
{
    Image_1D,
    Image_2D,
    Image_3D,
};

enum class ImageUsageType
{
    Texture,
    Render_Target,
    Transient_Render_Target,
    Swapchain_Image
};

enum class ImageDomain
{
    Physical,
    Transient, 
    LinearHostCached,
    LinearHost,

    Count
};

enum class Access : u32
{
    Indirect_Command_Read           = 1 << 0,
    Index_Read                      = 1 << 1,
    Vertex_Attribute_Read           = 1 << 2,
    Uniform_Read                    = 1 << 3,
    Input_Attachmnet_Read           = 1 << 4,
    Shader_Read                     = 1 << 5,
    Shader_Write                    = 1 << 6,
    Color_Attachment_Read           = 1 << 7,
    Color_Attachment_Write          = 1 << 8,
    Depth_Stencil_Attachment_Read   = 1 << 9,
    Depth_Stencil_Attachment_Write  = 1 << 10,
    Transfer_Read                   = 1 << 11,
    Transfer_Write                  = 1 << 12,
    Host_Read                       = 1 << 13,
    Host_Write                      = 1 << 14,
    Memory_Read                     = 1 << 15,
    Memory_Write                    = 1 << 16
};
using AccessFlags = u32;
std::string AccessFlagsToString(const AccessFlags& flags);

enum class RenderPassOp
{
    Clear_Depth_Stencil         = 1 << 0,
    Load_Depth_Stencil          = 1 << 1,
    Store_Depth_Stencil         = 1 << 2,
    Depth_Stencil_Read_Only     = 1 << 3,
    Enable_Transient_Store      = 1 << 4,
    Enable_Transient_Load       = 1 << 5,
};
using RenderPassOpFlags = u32;

/// <summary>
/// Define the three queues which can be used on the GPU.
/// </summary>
enum class GPUQueue
{
    GRAPHICS,
    COMPUTE,
    TRANSFER
};

enum class DescriptorType
{
    Sampler,
    Combined_Image_Sampler,
    Sampled_Image,
    Storage_Image,
    Uniform_Texel_Buffer,
    Storage_Texel_Buffer,
    Unifom_Buffer,
    Storage_Buffer,
    Uniform_Buffer_Dynamic,
    Storage_Buffer_Dyanmic,
    Input_Attachment,
};

enum class GPUResults
{
    Success = 0,
    Not_Ready = 1,
    Timeout = 2,
    Event_Set = 3,
    Event_Reset = 4,
    Incomplete = 5,
    Error_Out_Of_Host_Memory = -1,
    Error_Out_Of_Device_Memory = -2,
    Error_Initialization_Failed = -3,
    Error_Device_Lost = -4,
    Error_Memory_Map_Failed = -5,
    Error_Layer_Not_Present = -6,
    Error_Extension_Not_Present = -7,
    Error_Feature_Not_Present = -8,
    Error_Incompatiable_Driver = -9,
    Error_Too_Many_Objects = -10,
    Error_Format_Not_Supported = -11,
    Error_Fragmented_Pool = -12,
    Error_Unkown = -13,

    Error_Surface_Lost = -1000000000,
    Error_Suboptimal_Lost = 1000001003,
    Error_Out_Of_Data = -1000001004,
    Error_Incompatible_Display = -1000003001,
    Error_Native_Window_In_Use = -1000000001,
    Error_Validation_Failed = -1000011001,
    Error_Out_Of_Pool_Memory = 1000069000,

};

namespace Insight::Graphics
{
    enum class MaterialDrawMode
    {
        Transparent,
        Opaque
    };
    const MaterialDrawMode AllNaterialDrawModes[] = { MaterialDrawMode::Transparent, MaterialDrawMode::Opaque };
}